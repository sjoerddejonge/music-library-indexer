//
// Created by Sjoerd de Jonge on 03/04/2026.
//

#include <bitset>
#include <fstream>
#include <iostream>
#include "id3_parser.hpp"
#include "options.hpp"
#include "util/base64.hpp"
#include "util/helpers.hpp"
#include "nlohmann/json.hpp"

//
//      Internal implementation functions:
//

/**
 * @brief Parses the header of an ID3 tag.
 * @param fin Const reference to a std::ifstream at the start of the ID3 tag
 * @param options IndexOptions of which the option 'verbose' is used to enable/disable console output
 * @return Struct ID3Header containing the file_identifier, version, flags, and size
 */
static ID3Header parseId3Header(std::ifstream& fin, const IndexOptions& options);

/**
 * @brief Scans all supported ID3 frames of a song and writes them to the JSON.
 * @param fin Const reference to a std::ifstream
 * @param id3_header Header of the ID3 tag
 * @param options IndexOptions for 'verbose' and 'include_apic'
 * @return JSON that holds ID3 frame data of a song
 */
static nlohmann::json extractId3Frames(std::ifstream& fin, ID3Header id3_header, const IndexOptions& options);

/**
 * @brief Create an ID3 frame struct for the supported ID3 frames.
 * @param frame_header Header of the ID3 frame
 * @param data Byte vector with the data of the frame minus the header
 * @param options IndexOptions for 'include_apic' to toggle APIC frame inclusion in output
 * @return
 */
static std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& data, const IndexOptions& options);

nlohmann::json id3ToJson(std::ifstream& fin, const std::streampos &id3_pos, const IndexOptions& options) {
    // Move to start of ID3 tag
    fin.seekg(id3_pos);
    // Extract ID3 header.
    const ID3Header id3_header = parseId3Header(fin, options);
    // Extract ID3 frames and add to JSON.
    nlohmann::json song = extractId3Frames(fin, id3_header, options);
    // "ID3" file identifier in the header guarantees this is ID3v2.X
    song["id3_version"] = std::format("2.{}.{}", id3_header.version[0], id3_header.version[1]);
    return song;
}

static ID3Header parseId3Header(std::ifstream& fin, const IndexOptions& options) {
    ID3Header id3_header{};
    fin.read(reinterpret_cast<char*>(&id3_header), sizeof(id3_header));
    const std::bitset<8> flags{id3_header.flags};
    if (options.verbose) {
        std::cout << charsToStr(id3_header.file_identifier);
        std::cout << "v2." << static_cast<int>(id3_header.version[0]) << "." << static_cast<int>(id3_header.version[1]);
        std::cout << "  -  flags: " << flags;
        std::cout << "  -  size: " << id3_header.getSize() << "\n";
    }

    // If file has an extended header, skip past it.                                bit: 76543210
    // If flag b is set, which is bit 6, it has an extended header. (considering flags = abcd0000)
    if (id3_header.flags & (1 << 6 )) {
        std::array<uint8_t, 4> extended_header_size{};
        fin.read(reinterpret_cast<char*>(&extended_header_size), sizeof(extended_header_size));
        // Skip ahead extended_header_size bytes from current position:
        const uint32_t size_to_skip = (id3_header.version[0] >= 4)
            ? fromSynchsafe32(extended_header_size)
            : fromBigEndianInt(fromArrayToInt32(extended_header_size));
        fin.seekg(size_to_skip, std::ios_base::cur);
    }

    return id3_header;
}

static nlohmann::json extractId3Frames(std::ifstream& fin, const ID3Header id3_header, const IndexOptions& options) {
    nlohmann::json song;
    const uint32_t id3_size = id3_header.getSize();
    const int curr = fin.tellg(); // Current pos on the ifstream, just past the ID3 header

    // For every frame:
    while (fin.good() && fin.tellg() < curr + id3_size) {
        const bool synchsafe = id3_header.version[0] >= 4;
        // Create the frame header
        ID3FrameHeader id3_frame_header{};
        fin.read(reinterpret_cast<char*>(&id3_frame_header), sizeof(id3_frame_header));

        // Optional? Maybe this can be deleted after removing the couts in this function.
        // Exit when reaching padding bytes ( \0 = 00000000 )
        if (id3_frame_header.frame_id[0] == '\0') break;

        if (options.verbose) {
            const std::bitset<8> flag1{id3_frame_header.flags[0]};
            const std::bitset<8> flag2{id3_frame_header.flags[1]};
            std::cout << "frame: " << charsToStr(id3_frame_header.frame_id);
            std::cout << "\tsize: " << id3_frame_header.getSize(synchsafe);
            std::cout << " \tflags: " << flag1 << " " << flag2 << "\n";
        }

        // Read the frame data:
        const uint32_t size = id3_frame_header.getSize(synchsafe);
        std::vector<uint8_t> frame_data(size);
        fin.read(reinterpret_cast<char*>(frame_data.data()), id3_frame_header.getSize(synchsafe));

        if (const auto frame = makeFrame(id3_frame_header, frame_data, options)) {
                nlohmann::json frame_json = frame->toJson();
                if (!frame_json.empty()) {
                if (frame->isArrayType()) {
                    // Array type frames can hold multiple frames with the same ID (multiple COMMs, or APICs, for example)
                    song["id3_frames"][frame->header.frameIdToStr()].push_back(frame_json);
                } else {
                    song["id3_frames"].merge_patch(frame_json);
                }
            }
        }
    }
    return song;
}

std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& data, const IndexOptions& options) {
    const std::string id = frame_header.frameIdToStr();
    if (id == "TXXX") return std::make_unique<TXXX>(frame_header, data);
    if (id == "WXXX") return std::make_unique<WXXX>(frame_header, data);
    if (id == "COMM") return std::make_unique<COMM>(frame_header, data);
    if (id == "APIC" && options.include_apic) return std::make_unique<APIC>(frame_header, data);
    if (id[0] == 'T') return std::make_unique<TextInformationFrame>(frame_header, data);
    if (id[0] == 'W') return std::make_unique<UrlLinkFrame>(frame_header, data);
    return nullptr;
}

/**
 *  ==========================================
 *          ID3 Frame Structs methods:
 *  ==========================================
 */

// -------------------------------------
//      struct TextInformationFrame
// -------------------------------------

// Constructs a TextInformationFrame, throws error on empty data.
TextInformationFrame::TextInformationFrame(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never empty
    if (frame_data.empty()) throw std::runtime_error("TextInformationFrame: empty frame of type " + frame_header.frameIdToStr());
    header = frame_header;
    encoding = frame_data[0];
    value = parseTextInformationFrame(frame_data, encoding);
}

// Returns nlohmann::json for the frame under "frame_id"
nlohmann::json TextInformationFrame::toJson() const {
    nlohmann::json frame;
    if (!value.empty()) frame[header.frameIdToStr()] = value;
    return frame;
}

// (Private) Parse frame data into struct member variable(s).
std::string TextInformationFrame::parseTextInformationFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // TODO: Needs to account for scenarios where data is corrupted/not according to spec
    // Determine the start iterator as start of value field
    const auto it_start = std::next(frame_data.begin(), 1); // Skip 1 byte (encoding)
    auto [val, it_after_desc, little_endian] =
        readFieldToUtf8(it_start, frame_data.end(), text_encoding);
    return val;
}

// -------------------------------------
//              Struct TXXX
// -------------------------------------

// Constructs a TXXX, throws error on empty data.
TXXX::TXXX(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("TXXX: empty frame");
    header = frame_header;
    encoding = frame_data[0];
    auto [desc, val] = parseTXXXFrame(frame_data, encoding);
    description = std::move(desc);
    value = std::move(val);
}

// Returns nlohmann::json for the frame under "frame_id"
nlohmann::json TXXX::toJson() const {
    nlohmann::json frame;
    if (!description.empty() || !value.empty()) {
        frame["description"] = description;
        frame["value"] = value;
    }
    return frame;
}

bool TXXX::isArrayType() {
    return true;
}

// (Private) Parse frame data into struct member variable(s).
std::pair<std::string, std::string> TXXX::parseTXXXFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // text_encoding: 0 = ISO-8859-1, 1 = UTF-16, 2 = UTF-16BE, 3 = UTF-8
    // Determine the start iterator as start of description field
    const auto it_start = std::next(frame_data.begin(), 1); // Skip 1 byte (encoding)
    auto [desc, it_after_desc, little_endian] =
        readFieldToUtf8(it_start, frame_data.end(), text_encoding);
    // If there is no more data, end here.
    if (it_after_desc == frame_data.end()) return {desc, ""};
    auto [val, it_after_val, lit_endian] =
        readFieldToUtf8(it_after_desc, frame_data.end(), text_encoding, little_endian.value_or(true));
    return {desc, val};
}

// -------------------------------------
//              Struct COMM
// -------------------------------------

// Constructs a COMM, throws error on empty data.
COMM::COMM(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("COMM: empty frame");
    this->header = frame_header;
    encoding = frame_data[0];
    language = {static_cast<char>(frame_data[1]), static_cast<char>(frame_data[2]), static_cast<char>(frame_data[3])};
    auto [desc, val] = parseCOMMFrame(frame_data, encoding);
    description = std::move(desc);
    value = std::move(val);
}

// Returns nlohmann::json for the frame under "frame_id"
nlohmann::json COMM::toJson() const {
    nlohmann::json frame;
    if (!description.empty() || !value.empty()) {
        frame["language"] = charsToStr(language);
        frame["description"] = description;
        frame["comment"] = value;
    }
    return frame;
}

bool COMM::isArrayType() {
    return true;
}

// (Private) Parse frame data into struct member variable(s).
std::pair<std::string, std::string> COMM::parseCOMMFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // Determine the start iterator as start of description field
    const auto it_start = std::next(frame_data.begin(), 4); // Skip 4 bytes (encoding + lang)
    auto [desc, it_after_desc, little_endian] =
        readFieldToUtf8(it_start, frame_data.end(), text_encoding);
    // If there is no more data, end here.
    if (it_after_desc == frame_data.end()) return {desc, ""};
    auto [val, it_after_val, lit_endian] =
        readFieldToUtf8(it_after_desc, frame_data.end(), text_encoding, little_endian.value_or(true));
    return {desc, val};
}

// -------------------------------------
//              Struct APIC
// -------------------------------------

// Constructs an APIC, throws error on empty data.
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

// Returns nlohmann::json for the frame under "frame_id"
nlohmann::json APIC::toJson() const {
    nlohmann::json frame;
    if (!data.empty()) {
        frame["mime_type"] = mime_type;
        frame["picture_type"] = picture_type;
        frame["description"] = description;
        frame["picture_data"] = base64Encode(data);
    }
    return frame;
}

bool APIC::isArrayType() {
    return true;
}

// (Private) Parse frame data into struct member variable(s).
std::tuple<std::string, uint8_t, std::string, std::vector<uint8_t>> APIC::parseAPICFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
    // Text encoding      $xx
    // MIME type          <text string> $00
    // Picture type       $xx
    // Description        <text string according to encoding> $00 (00)
    // Picture data       <binary data>

    std::string mime{};
    uint8_t picture = 0;
    std::vector<uint8_t> picture_data{};

    // TODO: Refactor this code and check for safety with malformed data
    auto it_begin = frame_data.begin() + 1; // No BOM at start of file.
    auto it_end = std::find(it_begin, frame_data.end(), 0x00);
    mime = toUtf8(it_begin, it_end, 0); // Always ISO-8859-1 for MIME (int encoding = 0)

    it_begin = it_end;
    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {mime, picture, "", picture_data};
    // Always advance by 1 because MIME has 1 byte terminator
    std::advance(it_begin, 1);

    picture = *it_begin;

    std::advance(it_begin, 1);
    // If there is no more data, end here.
    if (it_begin == frame_data.end()) return {mime, picture, "", picture_data};

    auto [desc, it_after_desc, little_endian] = readFieldToUtf8(it_begin, frame_data.end(), text_encoding);

    // If there is no more data, end here.
    if (it_after_desc == frame_data.end()) return {mime, picture, desc, picture_data};

    it_end = frame_data.end();
    picture_data = std::vector<uint8_t>(it_after_desc, it_end);

    return {mime, picture, desc, picture_data};
}

// -------------------------------------
//          Struct UrlLinkFrame
// -------------------------------------

UrlLinkFrame::UrlLinkFrame(const ID3FrameHeader frame_header, const std::vector<uint8_t> &frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("UrlLinkFrame: empty frame");

    header = frame_header;
    auto [text, iter, opt] = readFieldToUtf8(frame_data.begin(), frame_data.end(), 0);
    url = text;
}

nlohmann::json UrlLinkFrame::toJson() const {
    nlohmann::json frame;
    if (!url.empty()) {
        frame[header.frameIdToStr()] = url;
    }
    return frame;
}

// -------------------------------------
//              Struct WXXX
// -------------------------------------

WXXX::WXXX(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
    // Data should never be empty
    if (frame_data.empty()) throw std::runtime_error("WXXX: empty frame");
    header = frame_header;
    encoding = frame_data[0];
    auto it_begin = frame_data.begin() + 1;
    auto [desc, it_after_field, opt] = readFieldToUtf8(it_begin, frame_data.end(), encoding);
    description = desc;
    auto [text, iter, opt2] = readFieldToUtf8(it_after_field, frame_data.end(), 0);
    url = text;
}

nlohmann::json WXXX::toJson() const {
    nlohmann::json frame;
    if (!url.empty()) {
        frame["description"] = description;
        frame["url"] = url;
    }
    return frame;
}

bool WXXX::isArrayType() {
    return true;
}
