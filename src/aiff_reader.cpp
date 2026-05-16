//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#include "aiff_reader.hpp"

#include <format>

#include "id3_parser.hpp"
#include "util/helpers.hpp"
#include <fstream>
#include <ios>
#include <iostream>

// Anonymous namespace hiding internal functions:
namespace {
struct ChunkHeader {
    std::array<char, 4> ck_id;   // (32 bits)
    int32_t ck_size;             // (32 bits)
};

// The FORM chunk as defined by AIFF spec. It is a container chunk for the other chunks.
struct FormChunk {
    std::array<char, 4> ck_id;       // Always 'FORM' (32 bits)
    int32_t ck_size;                 // The size of the data portion (32 bits)
    std::array<char, 4> form_type;   // What is in the FORM chunk, always 'AIFF' (32 bits)
};

#pragma pack(push, 1)
struct CommonChunk {
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
struct CommentChunk {

};
#pragma pack(pop)

// A comment chunk
#pragma pack(push, 1) // Probably unnecessary due to byte alignment
struct CommentHeader {
    uint32_t time_stamp;
    uint16_t marker_id;
    uint16_t count;
};
#pragma pack(pop)
} // namespace

// Reads the ifstream of an AIFF to the point start of the ID3 tag.
//
// Arguments:
// fin: The ifstream of an .aiff file.
// verbose: (Optional) bool to toggle console output.
aiff::Metadata aiff::scanFile(std::ifstream& fin, const bool verbose) {
    Metadata aiff_data;
    aiff_data.id3_pos = std::nullopt;
    if (fin) {
        FormChunk form_chunk{};
        fin.read(reinterpret_cast<char*>(&form_chunk), sizeof(form_chunk));
        form_chunk.ck_size = fromBigEndianInt(form_chunk.ck_size); // Byte swap from big endian to native endian

        if (verbose) {
            std::cout << "=== FORM chunk ===" << "\n";
            std::cout << "ckID: " << charsToStr(form_chunk.ck_id) << "\n";
            std::cout << "ckSize: " << form_chunk.ck_size << "\n";
            std::cout << "formType: " << charsToStr(form_chunk.form_type) << "\n";
        }

        // Loop through the file, extracting the ckID and ckSize of each chunk
        while (fin.good() && fin.tellg() < form_chunk.ck_size + 8) {
            ChunkHeader chunk_header{};
            fin.read(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
            chunk_header.ck_size = fromBigEndianInt(chunk_header.ck_size);

            const std::string ckID = charsToStr(chunk_header.ck_id);

            if (verbose) {
                std::cout << std::format("=== {} chunk ===", ckID)  << "\n";
                std::cout << "ckID: " << ckID << "\n";
                std::cout << "ckSize: " << chunk_header.ck_size << "\n";
            }

            if (ckID == "ID3 ") {
                if (verbose) std::cout << "Found ID3 tag." << "\n";
                aiff_data.id3_pos = fin.tellg();
            }

            // TODO: refactor: write these blocks into a function to be reused

            if (ckID == "NAME") {
                aiff_data.name.resize(chunk_header.ck_size); // Resize container
                fin.read(reinterpret_cast<std::istream::char_type *>(aiff_data.name.data()), chunk_header.ck_size);
                if (chunk_header.ck_size % 2 != 0) fin.seekg(1, std::ios_base::cur);
                continue;
            }

            if (ckID == "AUTH") {
                aiff_data.auth.resize(chunk_header.ck_size); // Resize container
                fin.read(reinterpret_cast<std::istream::char_type *>(aiff_data.auth.data()), chunk_header.ck_size);
                if (chunk_header.ck_size % 2 != 0) fin.seekg(1, std::ios_base::cur);
                continue;
            }

            if (ckID == "(c) ") {
                aiff_data.copyright.resize(chunk_header.ck_size); // Resize container
                fin.read(reinterpret_cast<std::istream::char_type *>(aiff_data.copyright.data()), chunk_header.ck_size);
                if (chunk_header.ck_size % 2 != 0) fin.seekg(1, std::ios_base::cur);
                continue;
            }

            if (ckID == "ANNO") {
                aiff_data.anno.resize(chunk_header.ck_size); // Resize container
                fin.read(reinterpret_cast<std::istream::char_type *>(aiff_data.anno.data()), chunk_header.ck_size);
                if (chunk_header.ck_size % 2 != 0) fin.seekg(1, std::ios_base::cur);
                continue;
            }

            if (ckID == "COMT") {
                // unsigned short numComments;
                // Comment comments[];
                uint16_t num_comments = 0;
                fin.read(reinterpret_cast<std::istream::char_type *>(&num_comments), sizeof(num_comments));
                num_comments = fromBigEndianInt(num_comments);
                // Loop through each comment
                for (uint16_t i = 0; i < num_comments; i++) {
                    CommentHeader comment_header{};
                    fin.read(reinterpret_cast<std::istream::char_type *>(&comment_header), sizeof(comment_header));
                    std::vector<uint8_t> text(fromBigEndianInt(comment_header.count));
                    fin.read(reinterpret_cast<std::istream::char_type *>(text.data()), fromBigEndianInt(comment_header.count));
                    aiff::Comment comment{
                        .marker_id = fromBigEndianInt(comment_header.marker_id),
                        .text = iso88591ToUtf8(text.begin(), text.end()),
                    };
                    aiff_data.comments.push_back(comment);
                    if (comment_header.count % 2 != 0) fin.seekg(1, std::ios_base::cur);
                }
                continue;
            }

            // Determine how far we skip ahead, which is equal to the size of data in the chunk.
            // If the ckSize is odd, there is a padding byte at the end not counted in ckSize.
            const int32_t skip = (chunk_header.ck_size % 2 == 0) ? chunk_header.ck_size : chunk_header.ck_size + 1;
            fin.seekg(skip, std::ios_base::cur);
            if (verbose) std::cout << "current position: " << fin.tellg() << "\n";
        }
    }
    return aiff_data;
}
