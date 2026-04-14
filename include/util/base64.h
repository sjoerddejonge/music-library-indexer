//
// Created by Sjoerd de Jonge on 14/04/2026.
//

#ifndef MLI_BASE64_H
#define MLI_BASE64_H
#include <array>
#include <iostream>
#include <string>
#include <vector>

// https://en.wikipedia.org/wiki/Base64

// Take 3 bytes (24 bits)
// Split them into 4 groups of 6 bits each.
// Map each 6 bits value to a character from a fixed alphabet of 64 characters.

// The Base64 character alphabet.
inline std::array base64 = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// The Base64URL character alphabet.
inline std::array base64URL = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};

inline constexpr char padding = '='; // The padding character for when data.size() % 3 != 0

// Encode a vector of bytes to a string using Base64. For Base64URL alphabet set bool url to true.
inline std::string base64Encode(std::vector<uint8_t> const& data, const bool url = false) {
    std::string result;
    const std::array<char, 64>& alphabet = url ? base64URL : base64; // Reference the array to avoid making copies.
    const char& pad = padding;
    int i;
    // Loop through data in sets of 3. If data.size() % 3 != 0, we do the last bytes after the loop.
    for (i = 0; i + 3 <= data.size(); i += 3) {
        // Construct four 6 bit integers (a,b,c,d) with the data from 3 characters:
        // a = the top 6 bits of b0
        // b = the bottom 2 bits of b0 combined with the top 4 bits of b1
        // c = the bottom 4 bits of b1 combined with the top 2 bits of b2
        // d = the bottom 6 bits of b2
        // Or, in other words:
        // byte 0: aaaaaabb
        // byte 1: bbbbcccc
        // byte 2: ccdddddd

        // Grab top 6 bits of byte0, shift right by 2.
        const uint8_t a = (data[i] & 0b11111100) >> 2;
        // Grab last 2 bits of byte0, shift left by 4, combine with top 4 bits of byte1 shifted right by 4.
        const uint8_t b = ((data[i] & 0b00000011) << 4) | ((data[i+1] & 0b11110000) >> 4);
        // Grab last 4 bits of byte1, shift left by 2, combine with top 2 bits of byte2, shifted right by 6.
        const uint8_t c = ((data[i+1] & 0b00001111 ) << 2) | ((data[i+2] & 0b11000000) >> 6);
        // Grab last 6 bits of byte2.
        const uint8_t d = ((data[i+2] & 0b00111111));
        result += alphabet[a];
        result += alphabet[b];
        result += alphabet[c];
        result += alphabet[d];
    }

    // When (data.size() % 3 != 0), we have to add padding.
    if ( data.size() % 3 == 2 ) {
        // 1 byte of padding needed for the last 2 bytes.
        const uint8_t a = (data[i] & 0b11111100) >> 2;
        const uint8_t b = ((data[i] & 0b00000011) << 4) | ((data[i+1] & 0b11110000) >> 4);
        const uint8_t c = ((data[i+1] & 0b00001111 ) << 2);
        result += alphabet[a];
        result += alphabet[b];
        result += alphabet[c];
        result += pad;
    }
    else if ( data.size() % 3 == 1 ) {
        // 2 bytes of padding needed for the last byte.
        const uint8_t a = (data[i] & 0b11111100) >> 2;
        const uint8_t b = ((data[i] & 0b00000011) << 4);
        result += alphabet[a];
        result += alphabet[b];
        result += pad;
        result += pad;
    }
    return result;
};

// Find the first index of a character in an array.
template <size_t N>
int indexOfChar(char c, std::array<char, N> arr) {
    return strchr(arr.data(), c)-arr.data();
}

// Decode a Base64 string to a vector of bytes. For Base64URL alphabet set bool url to true.
inline std::vector<uint8_t> base64Decode(std::string const& data, const bool url = false) {
    std::vector<uint8_t> result;
    const std::array<char, 64>& alphabet = url ? base64URL : base64;
    const char& pad = padding;
    int i;
    for (i = 0; i + 4 <= data.size(); i += 4) {
        const uint8_t a = indexOfChar(data[i], alphabet);
        const uint8_t b = indexOfChar(data[i+1], alphabet);
        const uint8_t c = indexOfChar(data[i+2], alphabet);
        const uint8_t d = indexOfChar(data[i+3], alphabet);

        // byte 0: aaaaaabb
        // byte 1: bbbbcccc
        // byte 2: ccdddddd
        uint8_t byte0 = (a & 0b00111111) << 2 | (b & 0b00110000) >> 4;
        uint8_t byte1 = (b & 0b00001111) << 4 | (c & 0b00111100) >> 2;
        uint8_t byte2 = (c & 0b00000011) << 6 | (d & 0b00111111);


        result.push_back(byte0);
        if (data[i+2] != pad) result.push_back(byte1); // Only include if not padding
        if (data[i+3] != pad) result.push_back(byte2); // Only include if not padding
    }

    return result;
}

#endif //MLI_BASE64_H
