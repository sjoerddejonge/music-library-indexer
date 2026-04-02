//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MUSIC_LIBRARY_INDEXER_AIFF_READER_H
#define MUSIC_LIBRARY_INDEXER_AIFF_READER_H
#include <array>
#include <cstdint>

// Audio IFF uses big-endian order, meaning the most significant byte is stored at the smallest memory address

void read(const std::string& filename);

// The FORM chunk as defined by AIFF spec, without the chunks field
struct formChunk {
    std::array<char, 4> ckID;       // Always 'FORM' (32 bits)
    int32_t ckSize;                 // The size of the data portion (32 bits)
    std::array<char, 4> formType;   // What is in the FORM chunk, always 'AIFF' (32 bits)
};

#endif //MUSIC_LIBRARY_INDEXER_AIFF_READER_H
