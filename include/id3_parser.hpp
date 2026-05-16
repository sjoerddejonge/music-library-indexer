//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MLI_ID3_PARSER_H
#define MLI_ID3_PARSER_H
#include <iosfwd>
#include <cstdint>
#include <map>
#include "options.hpp"
#include "util/base64.hpp"
#include "util/helpers.hpp"
#include "nlohmann/json.hpp"


// ID3v2 also uses big-endian order
// https://id3.org/id3v2.3.0#ID3_tag_version_2.3.0
// https://id3.org/id3v2.4.0-structure

/**
 * ID3 tag consists of
 * 1. A header
 * 2. Multiple frames consisting of:
 *  a. A header
 *  b. Frame data
 */

// TODO: Add errors to frame type constructors and parsing functions to account for malformed data
// TODO: Only call fromSynchsafe32() helper when ID3 header flag bit `a` is not set
    // TODO: Add ID3HeaderOptions struct to store ID3 header flags? (Or include in IndexOptions? Or just a bool)
    // > That way we can pass down information like the unsynchronization flag or ID3 version (v2.3.0 or v2.4.0)



// ====================================================================================================================
//
//      Public Functions
//
// ====================================================================================================================

/**
 * @brief Extract all information from ID3 tag and write to JSON.
 * @param fin Reference to the ifstream of the music file
 * @param id3_pos The std::streampos of the ID3 tag in the music file
 * @param options A struct with options for running the command. For default see include/options.hpp
 * @return JSON with all parsed ID3 tag information of a song
 */
nlohmann::json id3ToJson(std::ifstream& fin, const std::streampos &id3_pos, const IndexOptions& options);



// ====================================================================================================================
//
//      Internal Template Functions
//
// ====================================================================================================================

/**
 * @brief Find terminating double byte $00 00 in a vector of bytes.
 * @tparam Iterator A random access iterator.
 * @param begin Start Iterator of the vector.
 * @param end End Iterator of the vector
 * @return Either an Iterator at the null terminating byte or an Iterator at the end of the vector
 */
template <std::random_access_iterator Iterator>
static Iterator findTerminatingIterator(Iterator begin, Iterator end) {
    Iterator it = begin;
    while (it < end) {
        auto next = std::next(it, 1);
        if (next == end) break;
        if (*it == 0x00 && *next == 0x00) return it;

        std::advance(it, 2);
    }
    return end;
}

/**
 * @brief Reads a field of a frame to UTF-8 string. From the start to the end OR null terminator if found.
 * @param begin Start iterator of a data vector
 * @param end_of_vector End iterator of data vector
 * @param encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
 * @param little_endian Default = false. In case of UTF-16 with no BOM (which occurs only at first field), set this to true when
 * byte pairs are little endian
 * @return Tuple of @code[field_string, iterator_past_terminator, bool_little_endian]@endcode .
*/
template <std::input_iterator Iterator>
static std::tuple<std::string, Iterator, std::optional<bool>> readFieldToUtf8(Iterator begin, Iterator end_of_vector, int encoding, bool little_endian = false) {
    bool double_byte = (encoding == 1 || encoding == 2); // True for UTF-16 encodings which uses a byte pair per character
    // UTF-16 case: check for BOM (2 bytes) for endianness and skip BOM.
    if (encoding == 1) {
        if (std::next(begin, 1) != end_of_vector) {
            uint16_t possible_bom = (*begin << 8) | *std::next(begin, 1);
            if (possible_bom == 0xFFFE || possible_bom == 0xFEFF) {
                little_endian = (possible_bom == 0xFFFE);
                std::advance(begin, 2);
            }
        }
    }
    auto field_end = (double_byte)
                      ? findTerminatingIterator(begin, end_of_vector)
                      : std::find(begin, end_of_vector, 0x00);
    std::string desc = toUtf8(begin, field_end, encoding, little_endian);
    begin = field_end;
    if ((double_byte && std::next(begin, 2) >= end_of_vector)
        || (!double_byte && std::next(begin, 1) >= end_of_vector))
    {
        // End of vector, return Iterator end
        if (encoding == 1) return {desc, end_of_vector, little_endian}; // Only return endianness in case of UTF-16.
        return {desc, end_of_vector, std::nullopt};
    }
    std::advance(begin, (double_byte)? 2 : 1);
    if (encoding == 1) return {desc, begin, little_endian}; // Only return endianness in case of UTF-16.
    return {desc, begin, std::nullopt};
};

#endif //MLI_ID3_PARSER_H
