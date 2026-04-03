//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#pragma once
#include <bit>
#include <concepts>

#ifndef MUSIC_LIBRARY_INDEXER_HELPERS_H
#define MUSIC_LIBRARY_INDEXER_HELPERS_H

template <std::integral T>
T fromBigEndianInt(T value) {
    if constexpr (std::endian::native == std::endian::big) {
        return value;
    } else {
        return std::byteswap(value);
    }
}

inline uint32_t fromSynchsafe(const std::array<uint8_t, 4>& value) {
    const uint32_t a = (value[0] & 0x7F) << 21;
    const uint32_t b = (value[1] & 0x7F) << 14;
    const uint32_t c = (value[2] & 0x7F) << 7;
    const uint32_t d = (value[3] & 0x7F);
    const uint32_t result = a | b | c | d;
    return result;
}

inline float fromBigEndianFloat(const float value) {
    if constexpr (std::endian::native == std::endian::big) {
        return value;
    } else {
        auto asInt = std::bit_cast<uint32_t>(value);
        asInt = std::byteswap(asInt);
        return std::bit_cast<float>(asInt);
    }
}

#endif //MUSIC_LIBRARY_INDEXER_HELPERS_H
