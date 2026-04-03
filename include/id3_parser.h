//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MLI_ID3_PARSER_H
#define MLI_ID3_PARSER_H
#include <iosfwd>

// ID3v2 also uses big-endian order
// https://id3.org/id3v2.3.0#ID3_tag_version_2.3.0

void parseId3Header(std::ifstream& fin);

// Should be 10 bytes = 80 bits:
// ID3v2/file identifier   "ID3"
// ID3v2 version           $03 00
// ID3v2 flags             %abc00000
// ID3v2 size              4 * %0xxxxxxx
#include <array>
#pragma pack(push, 1)
struct id3TagHeader {
    std::array<char, 3> file_identifier;    // 24 bits "ID3"
    std::array<uint8_t, 2> version;      // 16 bits
    uint8_t flags;                          // 8 bits
    std::array<uint8_t, 4> size;            // 32 bits Synchsafe integer
};
#pragma pack(pop)

#endif //MLI_ID3_PARSER_H
