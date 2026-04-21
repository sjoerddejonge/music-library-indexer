//
// Created by Sjoerd de Jonge on 03/04/2026.
//

#include <bitset>
#include <fstream>
#include <iostream>
#include "id3_parser.h"
#include "util/base64.h"
#include "util/helpers.h"
#include "util/json.hpp"


// Append the ID3 tag to the JSON as item it.
//
// Arguments:
// - fin: Reference to the ifstream at the start of the ID3 tag.
// - verbose: Optional bool to enable console output, default false.
nlohmann::json id3ToJson(std::ifstream& fin, const bool verbose) {
    nlohmann::json song;
    // Extract ID3 header.
    const ID3Header id3_header = parseId3Header(fin);
    // Extract ID3 frames and add to JSON.
    extractId3Frames(fin, fromSynchsafe32(id3_header.size), song, verbose);
    return song;
}

// Accepts an fstream at the start of an ID3 tag and parses the header(s).
//
// Arguments:
// - fin: Reference to the ifstream at the start of the ID3 tag.
// - verbose: Optional bool to enable console output, default = false.
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

// Accepts an ifstream at past the ID3 header, scans ID3 frames and writes them to JSON.
//
// Arguments:
// - fin: Reference to the ifstream at the start of the first ID3 frame.
// - id3_size: The size of the ID3 tag, including the header.
// - song: The JSON to store the song's ID3 data in.
// - verbose: Optional bool to enable console output, default = false.
void extractId3Frames(std::ifstream& fin, const uint32_t id3_size, nlohmann::json& song, const bool verbose) {
    const int curr = fin.tellg(); // Current pos on the ifstream, just past the ID3 header

    // For every frame:
    while (fin.good() && fin.tellg() < curr + id3_size) {
        // Create the frame header
        ID3FrameHeader id3_frame_header{};
        fin.read(reinterpret_cast<char*>(&id3_frame_header), sizeof(id3_frame_header));

        // Optional? Maybe this can be deleted after removing the couts in this function.
        // Exit when reaching padding bytes ( \0 = 00000000 )
        if (id3_frame_header.frame_id[0] == '\0') break;

        if (verbose) {
            std::cout << "frame: " << charsToStr(id3_frame_header.frame_id);
            std::cout << ", size: " << fromSynchsafe32(id3_frame_header.size) << "\n";
        }

        // Read the frame data:
        const uint32_t size = fromSynchsafe32(id3_frame_header.size);
        std::vector<uint8_t> frame_data(size);
        fin.read(reinterpret_cast<char*>(frame_data.data()), fromSynchsafe32(id3_frame_header.size));

        if (const auto frame = makeFrame(id3_frame_header, frame_data)) {
            frame->toJson(song);
        }
    }
}

// Create an ID3 frame struct for the supported ID3 frames.
std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader header, const std::vector<uint8_t>& data) {
    const std::string id = header.frameIdToStr();
    if (id == "TXXX") return std::make_unique<TXXX>(header, data);
    if (id == "COMM") return std::make_unique<COMM>(header, data);
    if (id == "APIC") return std::make_unique<APIC>(header, data);
    if (id[0] == 'T') return std::make_unique<TextInformationFrame>(header, data);
    return nullptr;
}

/**
 *  ==========================
 *  ID3 Frame Structs methods:
 *  ==========================
 */

// -------------------------------------
//      struct TextInformationFrame
// -------------------------------------

// Constructs a TextInformationFrame, throws on error.
TextInformationFrame::TextInformationFrame(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never empty
    if (frame_data.empty()) throw std::runtime_error("TextInformationFrame: empty frame of type " + frame_header.frameIdToStr());
    header = frame_header;
    encoding = frame_data[0];
    value = parseTextInformationFrame(frame_data, encoding);
}

// Append this frame to the JSON.
void TextInformationFrame::toJson(nlohmann::json& song) const {
    if (!value.empty()) song[header.frameIdToStr()] = value;
}

// (Private) Parse frame data into struct member variable(s).
std::string TextInformationFrame::parseTextInformationFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // TODO: Needs to account for scenarios where data is corrupted/not according to spec
    const auto it_begin = frame_data.begin() + 1; // Skip first byte, used for encoder
    const auto it_end = std::find(it_begin, frame_data.end(), 0x00);
    // Passing iterators so I won't have to copy the vector
    std::string result = toUtf8(it_begin, it_end, text_encoding);
    return result;
}

// -------------------------------------
//              struct TXXX
// -------------------------------------

// Constructs a TXXX, throws on error.
TXXX::TXXX(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("TXXX: empty frame");
    header = frame_header;
    encoding = frame_data[0];
    auto [desc, val] = parseTXXXFrame(frame_data, encoding);
    description = std::move(desc);
    value = std::move(val);
}

// Append this frame to the JSON.
void TXXX::toJson(nlohmann::json& song) const {
    if (!description.empty() || !value.empty()) song["TXXX"][description] = value;
}

// (Private) Parse frame data into struct member variable(s).
std::pair<std::string, std::string> TXXX::parseTXXXFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // text_encoding: 0 = ISO-8859-1, 1 = UTF-16, 2 = UTF-16BE, 3 = UTF-8
    // For UTF-16 the terminating byte is double
    const bool is_double_byte = (text_encoding == 1 || text_encoding == 2);
    // TODO: Determine endianness from BOM for the UTF-16 case here

    // TODO: Refactor this code in new function that reads a field (up to null terminator)
    auto it_begin = (is_double_byte)
                        ? frame_data.begin() + 3
                        : frame_data.begin() + 1;
    auto it_end = (is_double_byte)
                      ? findTerminatingIterator(it_begin, frame_data.end())
                      : std::find(it_begin, frame_data.end(), 0x00);
    std::string desc = toUtf8(it_begin, it_end, text_encoding);

    // We continue from the end position:
    it_begin = it_end;

    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {desc, ""};

    // Advance it_begin to the next byte(s) to find the next part
    // TODO: Review this again to catch issues with malformed data (what if there is only one byte left?)
    std::advance(it_begin, (is_double_byte)? 2 : 1);
    it_end = (is_double_byte)
                 ? findTerminatingIterator(it_begin, frame_data.end())
                 : std::find(it_begin, frame_data.end(), 0x00);
    std::string val = toUtf8(it_begin, it_end, text_encoding);

    return {desc, val};
}

// -------------------------------------
//              struct COMM
// -------------------------------------

// Constructs a COMM, throws on error.
COMM::COMM(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("COMM: empty frame");
    this->header = frame_header;
    encoding = frame_data[0];
    language = {frame_data[1], frame_data[2], frame_data[3]};
    auto [desc, val] = parseCOMMFrame(frame_data, encoding);
    description = std::move(desc);
    value = std::move(val);
}

// Append this frame to the JSON.
void COMM::toJson(nlohmann::json& song) const {
    // TODO: Add language field to JSON
    if (!description.empty() || !value.empty()) song["COMM"][description] = value;
}

// (Private) Parse frame data into struct member variable(s).
std::pair<std::string, std::string> COMM::parseCOMMFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // TODO: Looks almost identical to parseTXXXFrame(), maybe combine or inherit?
    // For UTF-16 the terminating byte is double
    const bool is_double_byte = (text_encoding == 1 || text_encoding == 2);
    // TODO: Determine endianness from BOM for the UTF-16 case here

    // TODO: Refactor this code in new function that reads a field (up to null terminator)
    auto it_begin = (is_double_byte)
                        ? frame_data.begin() + 6      // UTF-16: skip 6 bytes, encoder (1) + BOM (2) + language (3)
                        : frame_data.begin() + 4;     // UTF-8 and ISO: skip 6 bytes, encoder (1) + language (3)
    auto it_end = (is_double_byte)
                      ? findTerminatingIterator(it_begin, frame_data.end())
                      : std::find(it_begin, frame_data.end(), 0x00);
    std::string desc = toUtf8(it_begin, it_end, text_encoding);

    // We continue from the end position:
    it_begin = it_end;

    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {desc, ""};

    // Advance it_begin to the next byte(s) to find the next part
    // TODO: Review this again to catch issues with malformed data (what if there is only one byte left?)
    std::advance(it_begin, (is_double_byte)? 2 : 1);
    it_end = (is_double_byte)
                 ? findTerminatingIterator(it_begin, frame_data.end())
                 : std::find(it_begin, frame_data.end(), 0x00);
    std::string val = toUtf8(it_begin, it_end, text_encoding);

    return {desc, val};
}

// -------------------------------------
//              Struct APIC
// -------------------------------------

// Constructs an APIC, throws on error.
APIC::APIC(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("APIC: empty frame");

    this->header = frame_header;
    encoding = frame_data[0];
    auto [mime, picture, desc, picture_data] = parseAPICFrame(frame_data, encoding);
    mime_type = std::move(mime);
    picture_type = picture;
    description = std::move(desc);
    data = std::move(picture_data);
}

// Append this frame to the JSON.
void APIC::toJson(nlohmann::json& song) const {
    if (data.empty()) return;
    song["APIC"]["MIME type"] = mime_type;
    song["APIC"]["Picture type"] = picture_type;
    song["APIC"]["Description"] = description;
    song["APIC"]["Data"] = base64Encode(data);
}

// (Private) Parse frame data into struct member variable(s).
std::tuple<std::string, uint8_t, std::string, std::vector<uint8_t>> APIC::parseAPICFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    std::string mime{};
    uint8_t picture = 0;
    std::string desc;
    std::vector<uint8_t> picture_data{};

    // if (text_encoding == 1 || text_encoding == 2) {
    //     std::cerr << "UTF-16 is not yet supported.\n";
    //     return {mime, picture, desc, picture_data};
    // }

    // For UTF-16 the terminating byte is double, except for MIME type which is always ISO-8859-1 encoding
    const bool is_double_byte = (text_encoding == 1 || text_encoding == 2);
    // TODO: Refactor this code and check for safety with malformed data
    auto it_begin = (is_double_byte)
                        ? frame_data.begin() + 3
                        : frame_data.begin() + 1;
    auto it_end = std::find(it_begin, frame_data.end(), 0x00);
    mime = toUtf8(it_begin, it_end, 0);

    it_begin = it_end;
    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {mime, picture, desc, picture_data};
    // TODO: This needs a check to see if it_end + 2 is a valid address
    std::advance(it_begin, (is_double_byte)? 2 : 1);

    picture = *it_begin;

    // TODO: This needs a check to see if it_end + 2 is a valid address
    std::advance(it_begin, (is_double_byte)? 2 : 1);
    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {mime, picture, desc, picture_data};

    it_end = (is_double_byte)
        ? findTerminatingIterator(it_begin, frame_data.end())
        : std::find(it_begin, frame_data.end(), 0x00);

    desc = toUtf8(it_begin, it_end, text_encoding);

    it_begin = it_end;
    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {mime, picture, desc, picture_data};
    // TODO: This needs a check to see if it_end + 2 is a valid address
    std::advance(it_begin, (is_double_byte)? 2 : 1);

    it_end = frame_data.end();
    picture_data = std::vector<uint8_t>(it_begin, it_end);

    return {mime, picture, desc, picture_data};
}
