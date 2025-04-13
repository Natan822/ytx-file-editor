#include <fstream>
#include <vector>
#include <filesystem>
#include "YtxFile.h"
#include "Utils.h"
#include <loguru.hpp>
#include <cmath>

YtxFile::YtxFile(std::string _path)
    : buffer{}
{
    if (_path.empty())
    {
        valid = false;
        return;
    }

    cleanPath(_path);

    path = _path;
    name = Utils::splitString(path, "/").back();

    valid = true;
}
YtxFile::~YtxFile() {}

bool YtxFile::comparePath(std::string compare)
{
    std::string copyCompare = compare;
    cleanPath(copyCompare);

    return copyCompare == path;
}

void YtxFile::cleanPath(std::string& _path)
{
    Utils::trim(_path);
    Utils::replaceAll(_path, "\\", "/");
}

bool YtxFile::isValid()
{
    return valid;
}

void YtxFile::load()
{
    if (!valid)
    {
        LOG_F(ERROR, "Unable to load file: Invalid file: %s", path.c_str());
        return;
    }
    

    LOG_F(INFO, "Loading file: %s", path.c_str());
    std::ifstream file(path, std::ios::binary);

    if (!file.good())
    {
        ABORT_F("Failed to open file: %s", path.c_str());
    }

    LOG_F(INFO, "File loaded: %s", name.c_str());

    LOG_F(INFO, "Loading file into buffer: %s", name.c_str());
    long size = std::filesystem::file_size(path);
    buffer.resize(size);

    file.read(reinterpret_cast<char *>(buffer.data()), size);
    LOG_F(INFO, "File loaded into buffer: %s", name.c_str());

    LOG_F(INFO, "Closing file: %s", name.c_str());
    file.close();
    LOG_F(INFO, "File closed: %s", name.c_str());

    loadHeaderValues();
    loadPofo();
    loadEntrySections();
    loadEntries();
}

void YtxFile::loadHeaderValues()
{
    LOG_F(INFO, "Loading header values ...");
    if (buffer.size() == 0)
    {
        LOG_F(ERROR, "File was not properly loaded: Size = 0");
        return;
    }

    if (buffer.size() <= ((long)Offset::ENTRY_SECTIONS_COUNT + 4))
    {
        LOG_F(ERROR, "File is invalid or not compatible: Could not find Entry Sections count.");
        return;
    }

    LOG_F(INFO, "Loading entry sections count ...");
    std::vector<std::byte> entrySectionsCountVec = Utils::getIntegerFromBuffer(buffer, (long)Offset::ENTRY_SECTIONS_COUNT);
    entrySectionsCount = Utils::bytesToIntBigEndian(entrySectionsCountVec);
    LOG_F(INFO, "Entry sections count loaded: %d", entrySectionsCount);
    
    LOG_F(INFO, "Loading POFO file address ...");
    std::vector<std::byte> pofoAddressVec = Utils::getIntegerFromBuffer(buffer, (long)Offset::POFO_FILE_ADDRESS);
    pofoAddress = Utils::bytesToIntBigEndian(pofoAddressVec);
    LOG_F(INFO, "POFO address loaded: 0x%x", pofoAddress);

    LOG_F(INFO, "Header values loaded.");
}

void YtxFile::loadPofo()
{
    LOG_F(INFO, "Loading POFO file ...");
    if (buffer.size() == 0)
    {
        LOG_F(ERROR, "File was not properly loaded. Size = 0");
        return;
    }

    if (pofoAddress <= 0)
    {
        LOG_F(ERROR, "Invalid POFO address: 0x%x. Unable to proceed.", pofoAddress);
        return;
    }

    if (buffer.size() <= pofoAddress)
    {
        LOG_F(ERROR, "File is invalid or not compatible: POFO address not reached 0x%x", pofoAddress);
        return;
    }

    pofo = std::vector<std::byte>(buffer.begin() + pofoAddress + 0x20, buffer.end());
    LOG_F(INFO, "POFO file loaded. Size: 0x%x", pofo.size());
}

void YtxFile::loadEntrySections()
{
    LOG_F(INFO, "Loading entry sections ...");
    for (int entrySectionIndex = 0; entrySectionIndex < entrySectionsCount; entrySectionIndex++)
    {
        int bufferAddress = (int)Offset::ENTRY_SECTIONS_INFO + (entrySectionIndex * ENTRY_SECTION_INFO_SIZE);

        int id = Utils::getIntValueFromBuffer(buffer, bufferAddress);
        int entriesCount = Utils::getIntValueFromBuffer(buffer, bufferAddress + 4);
        int address = Utils::getIntValueFromBuffer(buffer, bufferAddress + 8);

        entrySections.push_back(EntrySection{id, entriesCount, address});
        LOG_F(INFO, "Entry section loaded: ID = %x; Entries Count = %d; Address = 0x%x", id, entriesCount, address);
    }
    LOG_F(INFO, "All entry sections loaded.");
}

void YtxFile::loadEntries()
{
    for (int sectionIndex = 0; sectionIndex < entrySections.size(); sectionIndex++)
    {
        EntrySection* section = &entrySections.at(sectionIndex);
        int address = section->address + 0x20;

        LOG_F(INFO, "Loading entries from 0x%x, Entry section ID = %x", address, section->id);
        for (int entryIndex = 0; entryIndex < section->entriesCount; entryIndex++)
        {
            int id = Utils::getIntValueFromBuffer(buffer, address + (entryIndex * ENTRY_SIZE));
            int stringAddress = Utils::getIntValueFromBuffer(buffer, address + (entryIndex * ENTRY_SIZE) + 4) + 0x20;
            std::u16string u16string = Utils::readStringUtf16(buffer, stringAddress);
            std::string u8String = Utils::convertUtf16ToUtf8(u16string);

            Entry entry = {id, stringAddress, u8String};
            section->entries.push_back(entry);

            LOG_F(INFO, "Entry loaded: ID = %x, String address = 0x%x, String = %s", 
                entry.id, 
                entry.stringAddress, 
                entry._string.c_str());
        }
        LOG_F(INFO, "All entries loaded.");
    }
}

void YtxFile::saveChanges()
{
    reassemble();

    std::ofstream out(path + ".out", std::ios::binary);
    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    out.close();
}

void YtxFile::reassemble()
{
    LOG_F(INFO, "Reassembling file: %s", name.c_str());

    buffer.resize(0x28);

    rewriteEntrySectionsInfo();
    rewriteEntrySections();
    rewritePofo();

    buffer.insert(buffer.end(), pofo.begin(), pofo.end());

    LOG_F(INFO, "File reassembled: %s", name.c_str());
}

void YtxFile::rewriteEntrySectionsInfo()
{
    LOG_F(INFO, "Rewriting entry sections info on buffer header.");
    int sectionAddress = entrySections.at(0).address;
    for (int sectionIndex = 0; sectionIndex < entrySections.size(); sectionIndex++)
    {
        EntrySection* section = &entrySections.at(sectionIndex);

        std::vector<std::byte> id = Utils::intToByteBigEndian(section->id);
        buffer.insert(buffer.end(), id.begin(), id.end());

        std::vector<std::byte> entriesCount = Utils::intToByteBigEndian(section->entriesCount);
        buffer.insert(buffer.end(), entriesCount.begin(), entriesCount.end());

        section->address = sectionAddress;
        std::vector<std::byte> address = Utils::intToByteBigEndian(section->address);
        buffer.insert(buffer.end(), address.begin(), address.end());

        int stringBytes = getSectionStringsSize(sectionIndex);
        sectionAddress += (section->entries.size() * ENTRY_SIZE) + stringBytes;
    }
    LOG_F(INFO, "Entry sections rewritten: Buffer size after entry sections info: 0x%x", buffer.size());
}

void YtxFile::rewriteEntrySections()
{
    // Rewriting entry sections
    LOG_F(INFO, "Rewriting entry sections.");

    for (int sectionIndex = 0; sectionIndex < entrySections.size(); sectionIndex++)
    {
        EntrySection* section = &entrySections.at(sectionIndex);
        LOG_F(INFO, "Rewriting entry section: ID = %x; Address = 0x%x", section->id, section->address);

        int stringAddress = buffer.size() + (section->entriesCount * ENTRY_SIZE);
        for (int entryIndex = 0; entryIndex < section->entries.size(); entryIndex++)
        {
            Entry* entry = &section->entries.at(entryIndex);
            entry->stringAddress = stringAddress - 0x20;

            std::vector<std::byte> id = Utils::intToByteBigEndian(entry->id);
            std::vector<std::byte> stringAddressBytes = Utils::intToByteBigEndian(entry->stringAddress);

            buffer.insert(buffer.end(), id.begin(), id.end());
            buffer.insert(buffer.end(), stringAddressBytes.begin(), stringAddressBytes.end());

            int stringSize = (entry->_string.size() * 2) + 2;

            if (stringSize % 4 == 0 )
            {
                stringAddress += stringSize;
            }
            else
            {
                stringAddress += stringSize + 2;
            }
        }
        LOG_F(INFO, "Entry section rewritten: ID = %x; Buffer size = 0x%x", section->id, section->address, buffer.size());

        LOG_F(INFO, "Rewriting strings from entry section: ID = %x", section->id);
        for (int entryIndex = 0; entryIndex < section->entries.size(); entryIndex++)
        {
            Entry* entry = &section->entries.at(entryIndex);
            LOG_F(INFO, "Rewriting entry: ID = 0x%x; Address = 0x%x; String = %s", entry->id, entry->stringAddress, entry->_string.c_str());

            std::vector<std::byte> stringBytes = Utils::stringToBytes(entry->_string);
            buffer.insert(buffer.end(), stringBytes.begin(), stringBytes.end());
        }
        LOG_F(INFO, "Strings rewritten from entry section: ID = %x; Buffer size = 0x%x", section->id, buffer.size());
    }
    LOG_F(INFO, "Entry sections rewritten.");
}

void YtxFile::rewritePofo()
{
    LOG_F(INFO, "Rewriting POF0 file.");
    
    pofo.clear();

    // Update POFO's address at file header
    pofoAddress = buffer.size() - 0x20;
    Utils::writeIntToBuffer(buffer, pofoAddress, (int)Offset::POFO_FILE_ADDRESS);

    // POFO Magic number
    std::vector<std::byte> magic = {std::byte('P'), std::byte('O'), std::byte('F'), std::byte('0')};
    pofo.insert(pofo.end(), magic.begin(), magic.end());

    // Placeholder value for POFO size
    Utils::writeIntToBuffer(pofo, 0, pofo.size());

    std::vector<std::byte> sectionsInfo = {std::byte('A')};
    for (int i = 0; i < entrySectionsCount; i++)
    {
        sectionsInfo.push_back(std::byte('C'));
    }

    pofo.insert(pofo.end(), sectionsInfo.begin(), sectionsInfo.end());
    for (int sectionIndex = 0; sectionIndex < entrySectionsCount; sectionIndex++)
    {
        EntrySection section = entrySections.at(sectionIndex);

        std::vector<std::byte> entriesInfo;
        int initialIndex = (sectionIndex > 0) ? 1 : 0;
        for (int entry = initialIndex; entry < section.entriesCount; entry++)
        {
            entriesInfo.push_back(std::byte('B'));
        }
        pofo.insert(pofo.end(), entriesInfo.begin(), entriesInfo.end());

        // Not the last section
        if (sectionIndex < entrySectionsCount - 1)
        {
            int stringsSize = getSectionStringsSize(sectionIndex);
            int writeSize = stringsSize / 4;
            if (writeSize < 0x7FFE)
            {
                unsigned int writeValue = writeSize + 0x8002;
                std::vector<std::byte> writeValueBytes = {std::byte((writeValue & 0xFF00) >> 8), std::byte(writeValue & 0xFF)};
                pofo.insert(pofo.end(), writeValueBytes.begin(), writeValueBytes.end());
            }
            else
            {
                unsigned int writeValue = writeSize + 0xC0000002;
                Utils::writeIntToBuffer(pofo, writeValue, pofo.size());
            }
        }
    }
    // POF0 size without header
    int pofoSize = pofo.size() - 8;
    Utils::writeIntToBuffer(pofo, pofoSize, 4);

    LOG_F(INFO, "POF0 file rewritten: Size: 0x%x", pofo.size());
}

int YtxFile::getStringSize(std::string _string)
{
    int size = _string.size() * 2 + 2;
    if (size % 4 == 0)
    {
        return size;
    }
    return size + 2;
    
}

int YtxFile::getSectionStringsSize(int sectionIndex)
{
    EntrySection section = entrySections.at(sectionIndex);

    int sizeStrings = 0;
    for (Entry entry : section.entries)
    {

        sizeStrings += getStringSize(entry._string);
    }

    return sizeStrings;
}