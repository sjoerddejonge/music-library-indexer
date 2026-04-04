//
// Created by Sjoerd de Jonge on 03/04/2026.
//

#include "id3_parser.h"
#include <fstream>
#include <iostream>

#include "util/helpers.h"

void parseId3Header(std::ifstream& fin) {
    id3TagHeader id3_tag_header{};
    fin.read(reinterpret_cast<char*>(&id3_tag_header), sizeof(id3_tag_header));
    const std::string file_identifier{id3_tag_header.file_identifier.begin(), id3_tag_header.file_identifier.end()};
    std::cout << "=== ID3 Tag Header ===" << "\n";
    std::cout << "file_identifier: " << file_identifier << "\n";
    std::cout << "version: " << static_cast<int>(id3_tag_header.version[0]) << "." << static_cast<int>(id3_tag_header.version[1]) << "\n";
    const std::bitset<8> flags{id3_tag_header.flags};
    std::cout << "flags: " << flags << "\n";
    std::cout << "size: " << fromSynchsafe32(id3_tag_header.size) << "\n";
}
