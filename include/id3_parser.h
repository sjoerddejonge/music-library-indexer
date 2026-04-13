//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MLI_ID3_PARSER_H
#define MLI_ID3_PARSER_H
#include <iosfwd>

// ID3v2 also uses big-endian order
// https://id3.org/id3v2.3.0#ID3_tag_version_2.3.0
// https://id3.org/id3v2.4.0-structure

void parseId3Header(std::ifstream& fin);

#include <array>

#pragma pack(push, 1) // Prevent compiler from adding padding bytes.
// Header for the ID3 tag. Contains the version, flags, and size of the tag. Should be 10 bytes / 80 bits.
struct id3Header {
    std::array<char, 3> file_identifier;    // 24 bits "ID3"
    std::array<uint8_t, 2> version;         // 16 bits
    uint8_t flags;                          // 8 bits, first 3-4 bits are used as flags, others 0 (%abcd0000):
                                                // a Unsynchronization
                                                // b Extended header
                                                // c Experimental indicator
                                                // d Footer present
    std::array<uint8_t, 4> size;            // 32 bits Synchsafe integer (4 * %0xxxxxxx)
};
#pragma pack(pop)

#endif //MLI_ID3_PARSER_H
