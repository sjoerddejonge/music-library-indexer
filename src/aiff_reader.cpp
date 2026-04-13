//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#include "aiff_reader.h"
#include "id3_parser.h"
#include "util/helpers.h"

#include <fstream>
#include <ios>
#include <iostream>

void read(const std::string& filename) {
    std::ifstream fin{ filename, std::ios_base::binary }; // Create an if-stream to open the AIFF.
    if (fin) {
        formChunk form_chunk{};
        fin.read(reinterpret_cast<char*>(&form_chunk), sizeof(form_chunk));
        form_chunk.ckSize = fromBigEndianInt(form_chunk.ckSize); // Byte swap from big endian to native endian

        std::cout << "=== FORM chunk ===" << "\n";
        std::cout << "ckID: " << charsToStr(form_chunk.ckID) << "\n";
        std::cout << "ckSize: " << form_chunk.ckSize << "\n";
        std::cout << "formType: " << charsToStr(form_chunk.formType) << "\n";

        // Loop through the file, extracting the ckID and ckSize of each chunk
        while (fin.good() && fin.tellg() < form_chunk.ckSize + 8) {
            chunkHeader chunk_header{};
            fin.read(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
            chunk_header.ckSize = fromBigEndianInt(chunk_header.ckSize);

            const std::string ckID = charsToStr(chunk_header.ckID);
            std::cout << std::format("=== {} chunk ===", ckID)  << "\n";
            std::cout << "ckID: " << ckID << "\n";
            std::cout << "ckSize: " << chunk_header.ckSize << "\n";

            if (ckID == "ID3 ") {
                std::cout << "Passing fstream to id3_parser by reference" << "\n";
                parseId3Header(fin);
            }

            // Determine how far we skip ahead, which is equal to the size of data in the chunk.
            const int32_t skip = (chunk_header.ckSize % 2 == 0) ? chunk_header.ckSize : chunk_header.ckSize + 1;
            fin.seekg(skip, std::ios_base::cur);
            std::cout << "current position: " << fin.tellg() << "\n";
        }
    }
}
