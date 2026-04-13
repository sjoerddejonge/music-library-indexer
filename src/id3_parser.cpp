//
// Created by Sjoerd de Jonge on 03/04/2026.
//

#include "id3_parser.h"
#include <fstream>
#include <iostream>

#include "util/helpers.h"

//
// Accepts an fstream at the start of an ID3 tag and parses it.
//
void parseId3Header(std::ifstream& fin) {
    id3Header id3_tag_header{};
    fin.read(reinterpret_cast<char*>(&id3_tag_header), sizeof(id3_tag_header));
    std::cout << "=== ID3 Tag Header ===" << "\n";
    std::cout << "file_identifier: " << charsToStr(id3_tag_header.file_identifier) << "\n";
    std::cout << "version: " << static_cast<int>(id3_tag_header.version[0]) << "." << static_cast<int>(id3_tag_header.version[1]) << "\n";
    const std::bitset<8> flags{id3_tag_header.flags};
    std::cout << "flags: " << flags << "\n";
    std::cout << "size: " << fromSynchsafe32(id3_tag_header.size) << "\n";

    // If file has an extended header, skip past it.                76543210
    // Meaning flag b, which is bit 6, is set. (considering flags = abcd0000)
    if (id3_tag_header.flags & (1 << 6 )) {
        std::array<uint8_t, 4> extended_header_size{};
        fin.read(reinterpret_cast<char*>(&extended_header_size), sizeof(extended_header_size));
        // Skip ahead extended_header_size bytes from current position:
        fin.seekg(fromSynchsafe32(extended_header_size), std::ios_base::cur);
    }
}
