//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#include "aiff_reader.h"
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
        const std::string ckID{form_chunk.ckID.begin(), form_chunk.ckID.end()};
        const std::string formType{form_chunk.formType.begin(), form_chunk.formType.end()};
        std::cout << ckID << std::endl;
        std::cout << form_chunk.ckSize << std::endl;
        std::cout << formType << std::endl;
    }
}
