//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#pragma once
#include <bit>
#include <concepts>
#include <string>
#include <filesystem>

#ifndef MUSIC_LIBRARY_INDEXER_HELPERS_H
#define MUSIC_LIBRARY_INDEXER_HELPERS_H




// Convert any std::array<char, N> to std::string
template <size_t N>
std::string charsToStr(const std::array<char, N>& value) {
    const std::string str{value.begin(), value.end()};
    return str;
}

// Convert integer types from big endian to native endianness.
template <std::integral T>
T fromBigEndianInt(T value) {
    if constexpr (std::endian::native == std::endian::big) {
        // ReSharper disable once CppDFAUnreachableCode [need this for CLion to stop bugging me]
        return value;
    } else {
        return std::byteswap(value);
    }
}

// Convert a synchsafe array of 4 ints to a regular 32 bit int.
inline uint32_t fromSynchsafe32(const std::array<uint8_t, 4>& value) {
    const uint32_t a = (value[0] & 0b01111111) << 21;
    const uint32_t b = (value[1] & 0b01111111) << 14;
    const uint32_t c = (value[2] & 0b01111111) << 7;
    const uint32_t d = (value[3] & 0b01111111);
    const uint32_t result = a | b | c | d;
    return result;
}

// Convert float from big endian to native endianness.
inline float fromBigEndianFloat(const float value) {
    if constexpr (std::endian::native == std::endian::big) {
        // ReSharper disable once CppDFAUnreachableCode [need this for CLion to stop bugging me]
        return value;
    } else {
        auto asInt = std::bit_cast<uint32_t>(value);
        asInt = std::byteswap(asInt);
        return std::bit_cast<float>(asInt);
    }
}

// Find terminating double byte $00 00 in a vector of bytes.
template <std::input_iterator Iterator>
Iterator findTerminatingIterator(Iterator begin, Iterator end) {
    Iterator it = begin;
    while (it != end) {
        auto next = std::next(it, 1);
        if (next == end) break;
        if (*it == 0x00 && *next == 0x00) return it;

        std::advance(it, 2);
    }
    return end;
}

template <std::input_iterator Iterator>
std::string iso88591ToUtf8(Iterator begin, Iterator end) {
    std::string result;
    for (Iterator it = begin; it != end; ++it) {
        if (*it < 0b10000000) {
            result.push_back(static_cast<char> (*it));
        } else {
            result.push_back(static_cast<char> (0b11000000 | (*it >> 6)));
            result.push_back(static_cast<char> (0b10000000 | (*it & 0b00111111)));
        }
    }
    return result;
}

template <std::input_iterator Iterator>
std::string utf16ToUtf8(Iterator begin, Iterator end, bool little_endian) {
    std::string result;
    // TODO: Edge case - Uneven number of bytes.

    for (Iterator it = begin; it != end; std::advance(it, 2)) {
        // TODO: Edge case - std::next(it) does not exist. (Same as Edge case - Uneven number of bytes?)
        // 1. Construct the 16 bit (2 byte pair) number first, keeping endianness into account:
        const uint16_t byte_pair = (little_endian)
            ? *std::next(it, 1) << 8 | *it      // LE: First byte is low, second is high
            : (*it << 8) | *std::next(it, 1);   // BE: First byte is high, second is low

        uint32_t codepoint = 65533;      // Codepoint of the Replacement Character in case of error

        // 2. Determine the Unicode codepoint:
        if (byte_pair < 0xD800) {
            codepoint = byte_pair;       // Byte pair maps directly to codepoint
        }
        else if (byte_pair < 0xDC00) {
            // 0xD800 to 0xDBFF (0xDC00 to 0xDFFF is low surrogate)
            // ~~~Surrogate shit~~~
            // TODO: Edge case - No byte pair after current byte pair

            // W1 = 110110yyyyyyyyyy      // 0xD800 + yyyyyyyyyy
            // W2 = 110111xxxxxxxxxx      // 0xDC00 + xxxxxxxxxx
            // Construct high surrogate (from current byte pair):
            const uint16_t w1 = byte_pair;
            // Construct low surrogate (from next byte pair):
            const uint16_t w2 = (little_endian)
                ? *std::next(it, 3) << 8 | *std::next(it, 2)      // LE: First byte is low, second is high
                : (*std::next(it, 2) << 8) | *std::next(it, 3);   // BE: First byte is high, second is low
            if (w2 < 0xDC00 || w2 > 0xDFFF) {
                // TODO: Edge case - Low surrogate missing
                // TODO: Edge case - Low surrogate is actually a second high surrogate
            }
            const uint16_t w1_ = w1 - 0xD800;
            const uint16_t w2_ = w2 - 0xDC00;
            // 0000001111111111
            // 00000000000000000000001111111111
            // 00000000000000111111111100000000
            codepoint = ((w1_ << 10) | w2_) + 0x10000;

            // We already consumed the next byte pair, so advance iterator by 2 extra:
            std::advance(it, 2);
        }
        else if (byte_pair < 0xDFFF) {
            // TODO: Edge case - Low surrogate appears first
        }
        else if (byte_pair <= 0xFFFF) {
            // 0xE000 to 0xFFFF
            codepoint = byte_pair;       // Byte pair maps directly to codepoint
        }

        // 3. Translate the Unicode codepoint to UTF-8:
        if (codepoint  < 0b10000000) {
            // < 128
            result.push_back(static_cast<char> (codepoint));
        }
        else if (codepoint < 0b100000000000) {
            // 128 <= codepoint < 2048
            // 010000000000
            // 000000010000
            result.push_back(static_cast<char> (0b11000000 | (codepoint >> 6)));
            result.push_back(static_cast<char> (0b10000000 | (codepoint & 0b00111111)));
        }
        else if (codepoint < 0b10000000000000000) {
            // 2048 <= codepoint < 65536
            result.push_back(static_cast<char> (0b11100000 | (codepoint >> 12)));
            result.push_back(static_cast<char> (0b10000000 | ((codepoint >> 6) & 0b00111111)));
            result.push_back(static_cast<char> (0b10000000 | (codepoint & 0b00111111)));
        }
        else if (codepoint < 0x110000) {
            // 65536 <= codepoint < 1114111 (Unicode maximum)
            result.push_back(static_cast<char> (0b11110000 | (codepoint >> 18)));
            result.push_back(static_cast<char> (0b10000000 | ((codepoint >> 12) & 0b00111111)));
            result.push_back(static_cast<char> (0b10000000 | ((codepoint >> 6) & 0b00111111)));
            result.push_back(static_cast<char> (0b10000000 | (codepoint & 0b00111111)));
        }
        else {
            // 1114111 > codepoint or undefined error, use Replacement Character 0xEF 0xBF 0xBD
            result.push_back(static_cast<char> (0xEF));
            result.push_back(static_cast<char> (0xBF));
            result.push_back(static_cast<char> (0xBD));
        }
    }

    return result;
}

// Transform a vector with raw bytes to a string as Utf8 encoding.
//
// Arguments:
// - begin: Iterator starting at the beginning (or a point) in the data vector.
// - end: Iterator at the end (or a later point) of the data vector.
// - encoding: An integer specifying the encoding:
//      0 for ISO-8859-1
//      1 for UTF-16
//      2 for UTF-16BE
//      3 for UTF-8
// - little_endian: A bool used only for UTF-16 to indicate endianness, default = false
//   as recommended by section 4.3 https://www.rfc-editor.org/rfc/rfc2781.
template <std::input_iterator Iterator>
std::string toUtf8(Iterator begin, Iterator end, const int encoding, bool little_endian = false) {
    switch (encoding) {
        case 0:
            // ISO-8859-1
            return iso88591ToUtf8(begin, end);
        case 1:
            // UTF-16
            return utf16ToUtf8(begin, end, little_endian);
        case 2:
            // UTF-16BE
            return utf16ToUtf8(begin, end, false);
        case 3:
            // UTF-8
            return {begin, end};
        default:
            std::cerr << "Text encoding was not recognized.\n";
            break;
    }
    return {};
}

// Creates a unique, full export path by combining a file name and a path to a directory.
//
// Arguments:
// filename_in:     Filename (including extension) as a std::filesystem::path.
// path:            Path to the directory the file should be in as std::filesystem::path.
inline std::filesystem::path makeUniqueFilePath(const std::filesystem::path& filename_in, const std::filesystem::path& path) {
    std::filesystem::path filename_out = filename_in;
    // Check whether `path` is a directory.
    if (!std::filesystem::is_directory(path)) {
        std::cerr << "Error in createUniqueFilePath: Provide a valid path to a directory.\n";
        return filename_out;
    }
    std::filesystem::path full_path = path;
    full_path /= filename_out;
    int i = 0;
    while (std::filesystem::exists(full_path)) {
        filename_out = filename_in.stem();
        filename_out += "_";
        filename_out += std::to_string(i);
        filename_out += full_path.extension();
        full_path.replace_filename(filename_out.filename());
        ++i;
    }
    return full_path;
}

#endif //MUSIC_LIBRARY_INDEXER_HELPERS_H
