//
// Created by Sjoerd de Jonge on 03/04/2026.
//

#include <bitset>
#include <fstream>
#include <iostream>
#include "id3_parser.h"
#include "util/base64.h"
#include "util/helpers.h"


// Accepts an fstream at the start of an ID3 tag and parses the header(s).
ID3Header parseId3Header(std::ifstream& fin, const bool verbose) {
    ID3Header id3_tag_header{};
    fin.read(reinterpret_cast<char*>(&id3_tag_header), sizeof(id3_tag_header));
    if (verbose) {
        std::cout << "=== ID3 Tag Header ===" << "\n";
        std::cout << "file_identifier: " << charsToStr(id3_tag_header.file_identifier) << "\n";
        std::cout << "version: " << static_cast<int>(id3_tag_header.version[0]) << "." << static_cast<int>(id3_tag_header.version[1]) << "\n";
    }
    const std::bitset<8> flags{id3_tag_header.flags};
    if (verbose){
        std::cout << "flags: " << flags << "\n";
        std::cout << "size: " << fromSynchsafe32(id3_tag_header.size) << "\n";
    }

    // If file has an extended header, skip past it.                                bit: 76543210
    // If flag b is set, which is bit 6, it has an extended header. (considering flags = abcd0000)
    if (id3_tag_header.flags & (1 << 6 )) {
        std::array<uint8_t, 4> extended_header_size{};
        fin.read(reinterpret_cast<char*>(&extended_header_size), sizeof(extended_header_size));
        // Skip ahead extended_header_size bytes from current position:
        fin.seekg(fromSynchsafe32(extended_header_size), std::ios_base::cur);
    }

    return id3_tag_header;
}

// Accepts an fstream at past the ID3 header and scans the location of the frames.
std::map<std::string, std::vector<std::string>> extractId3Frames(std::ifstream& fin, const uint32_t tag_size, bool verbose) {
    std::map<std::string, std::vector<std::string>> frames;
    const int curr = fin.tellg(); // Current pos on the ifstream
    while (fin.good() && fin.tellg() < curr + tag_size) {
        ID3FrameHeader id3_frame_header{};
        fin.read(reinterpret_cast<char*>(&id3_frame_header), sizeof(id3_frame_header));

        // Optional? Maybe this can be deleted after removing the couts in this function.
        // Exit when reaching padding bytes ( \0 = 00000000 )
        if (id3_frame_header.frame_id[0] == '\0') break;

        if (verbose) {
            std::cout << "frame: " << charsToStr(id3_frame_header.frame_id);
            std::cout << ", size: " << fromSynchsafe32(id3_frame_header.size) << "\n";
        }

        const uint32_t size = fromSynchsafe32(id3_frame_header.size);
        std::vector<uint8_t> frame_data(size);
        fin.read(reinterpret_cast<char*>(frame_data.data()), fromSynchsafe32(id3_frame_header.size));

        ID3Frame id3_frame{
            id3_frame_header,
            std::move(frame_data),
        };

        std::string frame{};
        // If frame_id starts with a "T" (i.e. "TXXX"), it is a text frame
        if (charsToStr(id3_frame.header.frame_id)[0] == 'T') {
            frame = readTextFrameData(id3_frame);
        } else {
            //
        }
        // fin.read(reinterpret_cast<char*>(buffer.data()), size);
        if (charsToStr(id3_frame.header.frame_id) == "APIC") frame = base64Encode(id3_frame.data);
        frames[charsToStr(id3_frame.header.frame_id)].push_back(frame);
    }
    return frames;
}

std::string readTextFrameData(const ID3Frame &frame) {
    // Read first byte for encoding
    const uint8_t encoding = frame.data[0];

    // $00   ISO-8859-1 [ISO-8859-1]. Terminated with $00.
    // $01   UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM. All
    //       strings in the same frame SHALL have the same byteorder.
    //       Terminated with $00 00.
    // $02   UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM.
    //       Terminated with $00 00.
    // $03   UTF-8 [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.

    auto start = frame.data.begin();
    auto end = frame.data.end();

    switch (encoding) {
        case 0:
            // $00   ISO-8859-1 [ISO-8859-1]. Terminated with $00.
            start = frame.data.begin() + 1;
            end = frame.data.end();
            break;
        case 1:
            // $01   UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM. All
            //       strings in the same frame SHALL have the same byteorder.
            //       Terminated with $00 00.
            start = frame.data.begin() + 1;
            end = frame.data.end()-2;
            break;
        case 2:
            // $02   UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM.
            //       Terminated with $00 00.
            start = frame.data.begin() + 1;
            end = frame.data.end()-2;
            break;
        case 3:
            // $03   UTF-8 [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.
            start = frame.data.begin() + 1;
            end = frame.data.end()-1;
            break;
        default:
            break;
    }

    std::cout << "encoding: " << +encoding << "\n";
    std::string frame_string(start, end);
    return frame_string;
}


