//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#pragma once
#include <bit>
#include <concepts>
#include <string>

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
        // ReSharper disable once CppDFAUnreachableCode
        return value;
    } else {
        return std::byteswap(value);
    }
}

// Convert a synchsafe array of 4 ints to a regular 32 bit int
inline uint32_t fromSynchsafe32(const std::array<uint8_t, 4>& value) {
    const uint32_t a = (value[0] & 0x7F) << 21;
    const uint32_t b = (value[1] & 0x7F) << 14;
    const uint32_t c = (value[2] & 0x7F) << 7;
    const uint32_t d = (value[3] & 0x7F);
    const uint32_t result = a | b | c | d;
    return result;
}

// Convert float from big endian to native endianness.
inline float fromBigEndianFloat(const float value) {
    if constexpr (std::endian::native == std::endian::big) {
        // ReSharper disable once CppDFAUnreachableCode
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
        if (*it < 0x80) {
            result.push_back(static_cast<char> (*it));
        } else {
            result.push_back(static_cast<char> (0xc0 | (*it >> 6)));
            result.push_back(static_cast<char> (0x80 | (*it & 0x3f)));
        }
    }
    return result;
}

// inline std::string utf16ToUtf8(const std::vector<uint8_t>& data) {
//     std::string result;
//     return result;
// }
//
// inline std::string utf16beToUtf8(const std::vector<uint8_t>& data) {
//     std::string result;
//     return result;
// }

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
template <std::input_iterator Iterator>
std::string toUtf8(Iterator begin, Iterator end, const int encoding) {
    switch (encoding) {
        case 0:
            // ISO-8859-1
            return iso88591ToUtf8(begin, end);
        case 1:
            // UTF-16
            // TODO: Add UTF-16 support.
            std::cerr << "UTF-16 text is not yet supported!\n";
            // TODO: Limit console output. Is this the place for a throw()?
            break;
        case 2:
            // UTF-16BE
            // TODO: Add UTF-16BE support.
            std::cerr << "UTF-16BE text is not yet supported.\n";
            break;
        case 3:
            // UTF-8
            return {begin, end};
        default:
            std::cerr << "Text encoding was not recognized.\n";
            break;
    }
    return {};
}

#endif //MUSIC_LIBRARY_INDEXER_HELPERS_H
