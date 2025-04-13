#include "Utils.h"
#include <cstdint>
#include <locale>
#include <codecvt>

namespace Utils
{
    int bytesToIntBigEndian(std::vector<std::byte> bytes)
    {
        if (bytes.size() != 4)
        {
            return 0;
        }

        return 
            std::to_integer<int>(bytes.at(0)) << 24 |
            std::to_integer<int>(bytes.at(1)) << 16 |
            std::to_integer<int>(bytes.at(2)) << 8 |
            std::to_integer<int>(bytes.at(3));
    }

    std::vector<std::byte> intToByteBigEndian(int value)
    {
        std::vector<std::byte> result;
        
        result.push_back(std::byte((value & 0xFF000000) >> 24));
        result.push_back(std::byte((value & 0x00FF0000) >> 16));
        result.push_back(std::byte((value & 0x0000FF00) >> 8));
        result.push_back(std::byte(value & 0x000000FF));

        return result;
    }

    std::vector<std::byte> getIntegerFromBuffer(std::vector<std::byte> buffer, long offset)
    {
        return std::vector<std::byte>(buffer.begin() + offset, buffer.begin() + offset + 4);
    }

    std::vector<std::string> splitString(std::string _string, std::string delimiter)
    {
        std::vector<std::string> result;

        int start = 0;
        int index = _string.find(delimiter, start);
        while (index != std::string::npos)
        {
            result.push_back(_string.substr(start, index - start));

            start = index + delimiter.size();
            index = _string.find(delimiter, start);
        }
        result.push_back(_string.substr(start, _string.size() - start));

        return result;
    }

    void replaceAll(std::string &_string, std::string from, std::string to)
    {
        if (from.empty())
            return;

        int index = _string.find(from, 0);
        while (index != std::string::npos)
        {
            _string.replace(index, from.size(), to);
            index = _string.find(from, 0);
        }
    }

    void ltrim(std::string& _string)
    {
        int index = _string.find_first_not_of(" \t\n\r\f\v");
        _string = _string.substr(index, _string.size() - index);
    }

    void rtrim(std::string& _string)
    {
        int end = _string.find_last_not_of(" \t\n\r\f\v");
        _string = _string.substr(0, end + 1);
    }

    void trim(std::string& _string)
    {
        ltrim(_string);
        rtrim(_string);
    }

    std::u16string readStringUtf16(std::vector<std::byte> buffer, long offset)
    {
        if (buffer.size() <= offset)
        {
            return nullptr;
        }

        std::u16string result;

        uint16_t utf16Char = (uint16_t)(buffer.at(offset) << 8 | buffer.at(offset + 1));
        while (utf16Char != 0)
        {
            result.push_back(utf16Char);

            offset += 2;
            utf16Char = (uint16_t)(buffer.at(offset) << 8 | buffer.at(offset + 1));
        }

        return result;
    }

    int getIntValueFromBuffer(std::vector<std::byte> buffer, long offset)
    {
        return bytesToIntBigEndian(getIntegerFromBuffer(buffer, offset));
    }

    std::string convertUtf16ToUtf8(std::u16string sourceString)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.to_bytes(sourceString);
    }

    std::u16string convertUtf8ToUtf16(std::string sourceString)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.from_bytes(sourceString);
    }
    
    std::vector<std::byte> stringToBytes(std::u16string _string)
    {
        std::vector<std::byte> result;
        for (char16_t _char : _string)
        {
            result.push_back(std::byte((_char & 0xFF00) >> 8));
            result.push_back(std::byte(_char & 0x00FF));
        }
        // Null terminator
        result.push_back(std::byte(0));
        result.push_back(std::byte(0));

        // Everu string's size must be divisible by 4
        if (result.size() % 4 != 0)
        {
            result.push_back(std::byte(0));
            result.push_back(std::byte(0));
        }

        return result;
    }

    void writeIntToBuffer(std::vector<std::byte> &buffer, unsigned int value, int offset)
    {
        if (offset > buffer.size())
        {
            return;
        }
        
        std::vector<std::byte> valueBytes = intToByteBigEndian(value);
        if (offset == buffer.size())
        {
            buffer.insert(buffer.end(), valueBytes.begin(), valueBytes.end());
            return;
        }

        std::copy(valueBytes.begin(), valueBytes.end(), (buffer.begin() + offset));
    }
}