#pragma once

#include <vector>
#include <string>

namespace Utils
{
    // Convert 4 bytes to integer in big endian
    int bytesToIntBigEndian(std::vector<std::byte> value);

    // Convert an integer to 4 bytes in big endian
    std::vector<std::byte> intToByteBigEndian(int value);

    // Retrieve a 4-byte value from a buffer at a given offset
    std::vector<std::byte> getIntegerFromBuffer(std::vector<std::byte> buffer, long offset);

    // Just a regular split function for strings
    std::vector<std::string> splitString(std::string _string, std::string delimiter = " ");

    // Replace all ocurrences of "from" with "to" in a given string
    void replaceAll(std::string& _string, std::string from, std::string to);

    // Trim whitespaces
    void ltrim(std::string& _string); // Left
    void rtrim(std::string& _string); // RIght
    void trim(std::string& _string); // Left and Right

    // Read a UTF-16 string from a buffer at a given offset
    std::u16string readStringUtf16(std::vector<std::byte> buffer, long offset);

    // Same as getIntegerFromBuffer but returns an integer right away instead of bytes
    int getIntValueFromBuffer(std::vector<std::byte> buffer, long offset);

    // Convert a UTF-16 string to UTF-8
    std::string convertUtf16ToUtf8(std::u16string _string);
    // Convert a UTF-8 string to UTF-16
    std::u16string convertUtf8ToUtf16(std::string _string);

    // Convert a string to bytes as UTF-16 and make its size in bytes divisible by 4(required in .ytx files)
    std::vector<std::byte> stringToBytes(std::u16string _string);

    // Write an integer to a buffer in big endian given an offset
    void writeIntToBuffer(std::vector<std::byte> &buffer, unsigned int value, int offset);
    
}