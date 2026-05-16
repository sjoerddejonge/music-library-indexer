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

// Reads the ifstream of an AIFF to the point start of the ID3 tag.
//
// Arguments:
// fin: The ifstream of an .aiff file.
// verbose: (Optional) bool to toggle console output.
aiff::Metadata aiff::scanFile(std::ifstream& fin, const bool verbose) {
    aiff::Metadata aiff_data;
    aiff_data.id3_pos = std::nullopt;
    if (fin) {
        aiff::FormChunk form_chunk{};
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
            aiff::ChunkHeader chunk_header{};
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
                    aiff::CommentHeader comment_header{};
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
