//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#include "aiff_reader.h"

#include <format>

#include "id3_parser.h"
#include "util/helpers.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <map>

// Reads the ifstream of an AIFF to the point start of the ID3 tag.
//
// Arguments:
// fin: The ifstream of an .aiff file.
// verbose: (Optional) bool to toggle console output.
void locateId3(std::ifstream& fin, const bool verbose) {
    if (fin) {
        formChunk form_chunk{};
        fin.read(reinterpret_cast<char*>(&form_chunk), sizeof(form_chunk));
        form_chunk.ckSize = fromBigEndianInt(form_chunk.ckSize); // Byte swap from big endian to native endian

        if (verbose) {
            std::cout << "=== FORM chunk ===" << "\n";
            std::cout << "ckID: " << charsToStr(form_chunk.ckID) << "\n";
            std::cout << "ckSize: " << form_chunk.ckSize << "\n";
            std::cout << "formType: " << charsToStr(form_chunk.formType) << "\n";
        }

        // Loop through the file, extracting the ckID and ckSize of each chunk
        while (fin.good() && fin.tellg() < form_chunk.ckSize + 8) {
            chunkHeader chunk_header{};
            fin.read(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
            chunk_header.ckSize = fromBigEndianInt(chunk_header.ckSize);

            const std::string ckID = charsToStr(chunk_header.ckID);

            if (verbose) {
                std::cout << std::format("=== {} chunk ===", ckID)  << "\n";
                std::cout << "ckID: " << ckID << "\n";
                std::cout << "ckSize: " << chunk_header.ckSize << "\n";
            }

            if (ckID == "ID3 ") {
                if (verbose) std::cout << "Found ID3 tag." << "\n";
                // Terminate function
                return;
            }

            // Determine how far we skip ahead, which is equal to the size of data in the chunk.
            const int32_t skip = (chunk_header.ckSize % 2 == 0) ? chunk_header.ckSize : chunk_header.ckSize + 1;
            fin.seekg(skip, std::ios_base::cur);
            if (verbose) std::cout << "current position: " << fin.tellg() << "\n";
        }
    }
}
