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
    std::array<char, 4> ck_id;   // (32 bits)
    int32_t ck_size;             // (32 bits)
};

// The FORM chunk as defined by AIFF spec. It is a container chunk for the other chunks.
struct formChunk {
    std::array<char, 4> ck_id;       // Always 'FORM' (32 bits)
    int32_t ck_size;                 // The size of the data portion (32 bits)
    std::array<char, 4> form_type;   // What is in the FORM chunk, always 'AIFF' (32 bits)
};

#pragma pack(push, 1)
struct commonChunk {
    std::array<char, 4> ck_id;              // Always 'COMM' (32 bits)
    int32_t ck_size;                        // Size of the data portion of the chunk, always 18 (32 bits)
    int16_t num_channels;                   // Number of channels (*), 1 for mono, 2 for stereo, etc. (16 bits)
    uint32_t num_sample_frames;             // Number of samples in Sound Data Chunk (32 bits)
    int16_t sample_size;                    // Number of bits in each sample point (16 bits)
    std::array<uint8_t, 10> sample_rate;    // Sample rate in sample frames per second (80 bits)

    // (*) refer to the AIFF documentation (titled: 'Audio Interchange File Format: "AIFF"', Apple 1989)
    // for the distribution of channels in multichannel files. For stereo channel 1 is left and 2 is right.
};
#pragma pack(pop)

// COMT chunk
#pragma pack(push, 1)
struct commentChunk {
    
};
#pragma pack(pop)

// A comment chunk
#pragma pack(push, 1) // Probably unnecessary due to byte alignment
struct aiffCommentHeader {
    uint32_t time_stamp;
    uint16_t marker_id;
    uint16_t count;
};
#pragma pack(pop)

struct aiffComment {
    std::uint16_t marker_id;
    std::string text;
};

// Metadata to be extracted from the AIFF file
struct aiffData {
    std::vector<uint8_t> name;                  // Name chunk
    std::vector<uint8_t> auth;                  // Author chunk
    std::vector<uint8_t> copyright;             // Copyright chunk
    std::vector<uint8_t> anno;                  // Annotation chunk
    std::vector<aiffComment> comments;          // Comment chunk(s)
    std::optional<std::streampos> id3_pos;      // Position of the ID3 chunk (start of data part)
};

aiffData scanFile(std::ifstream& fin, bool verbose = false);

#endif //MLI_AIFF_READER_H
