//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MLI_AIFF_READER_H
#define MLI_AIFF_READER_H
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

// Audio IFF uses big-endian order, meaning the most significant byte is stored at the smallest memory address

struct chunkHeader {
    std::array<char, 4> ckID;   // (32 bits)
    int32_t ckSize;             // (32 bits)
};

// The FORM chunk as defined by AIFF spec. It is a container chunk for the other chunks.
struct formChunk {
    std::array<char, 4> ckID;       // Always 'FORM' (32 bits)
    int32_t ckSize;                 // The size of the data portion (32 bits)
    std::array<char, 4> formType;   // What is in the FORM chunk, always 'AIFF' (32 bits)
};

#pragma pack(push, 1)
struct commonChunk {
    std::array<char, 4> ckID;           // Always 'COMM' (32 bits)
    int32_t ckSize;                     // Size of the data portion of the chunk, always 18 (32 bits)
    int16_t numChannels;                // Number of channels (*), 1 for mono, 2 for stereo, etc. (16 bits)
    uint32_t numSampleFrames;           // Number of samples in Sound Data Chunk (32 bits)
    int16_t sampleSize;                 // Number of bits in each sample point (16 bits)
    std::array<uint8_t, 10> sampleRate; // Sample rate in sample frames per second (80 bits)

    // (*) refer to the AIFF documentation (titled: 'Audio Interchange File Format: "AIFF"', Apple 1989)
    // for the distribution of channels in multichannel files. For stereo channel 1 is left and 2 is right.
};
#pragma pack(pop)

void locateId3(std::ifstream& fin, bool verbose = false);

#endif //MLI_AIFF_READER_H
