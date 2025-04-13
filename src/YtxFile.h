#pragma once

#include <vector>
#include <string>

struct Entry
{
    int id;
    int stringAddress; // Address of the string in the file - 0x20
    std::string _string;
};

struct EntrySection
{
    int id;
    int entriesCount;
    int address;
    std::vector<Entry> entries;
};

class YtxFile
{

// Size of an entry in bytes
const int ENTRY_SIZE = 8;
// Size of an entry section info in bytes(EntrySection struct)
const int ENTRY_SECTION_INFO_SIZE = 12;

// Common offsets in .ytx files
enum class Offset
{
    POFO_FILE_ADDRESS = 0x1C,
    ENTRY_SECTIONS_COUNT = 0x20,
    ENTRY_SECTIONS_INFO = 0x28
};

public:
    std::vector<std::byte> buffer;
    std::vector<std::byte> pofo;
    std::vector<EntrySection> entrySections;

    YtxFile(std::string filePath);
    ~YtxFile();

    bool comparePath(std::string compare);
    bool isValid();
    void load();
    void saveChanges();

private:
    std::string name;
    std::string path;
    bool valid;
    bool hasBackup;

    int pofoAddress{};
    int entrySectionsCount{};

    void cleanPath(std::string& _path);

    void backupFile();
    void saveFile();

    void loadPofo();
    void loadHeaderValues();
    void loadEntrySections();
    void loadEntries();

    // Get the actual size in bytes occupied by a string in a file
    int getStringSize(std::string _string);
    // Get the total amount of bytes occupied by strings in a given section
    int getSectionStringsSize(int sectionIndex);

    void reassemble();

    void rewriteEntrySectionsInfo();
    void rewriteEntrySections();
    void rewritePofo();
};