//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MLI_ID3_PARSER_H
#define MLI_ID3_PARSER_H
#include <iosfwd>
#include <array>
#include <cstdint>
#include <vector>
#include <map>
#include <variant>

#include "util/base64.h"
#include "util/helpers.h"
#include "util/json.hpp"


// ID3v2 also uses big-endian order
// https://id3.org/id3v2.3.0#ID3_tag_version_2.3.0
// https://id3.org/id3v2.4.0-structure

// TODO: Refactor frame type structs to reduce duplicate code (inheritance?)
// TODO: Add errors to frame type constructors and parsing functions to account for malformed data

// Prevent compiler from adding padding bytes.
#pragma pack(push, 1)
// Header for the ID3 tag. Contains the version, flags, and size of the tag. Should be 10 bytes / 80 bits.
struct ID3Header {
    std::array<char, 3> file_identifier;    // 24 bits "ID3"
    std::array<uint8_t, 2> version;         // 16 bits
    uint8_t flags;                          // 8 bits, first 3-4 bits are used as flags, others 0 (%abcd0000):
                                                // a Unsynchronization
                                                // b Extended header
                                                // c Experimental indicator
                                                // d Footer present
    std::array<uint8_t, 4> size;            // 32 bits Synchsafe integer (4 * %0xxxxxxx)
};
#pragma pack(pop)

#pragma pack(push, 1)
// The header of the ID3 Frame.
struct ID3FrameHeader {
    std::array<char, 4> frame_id;           // 32 bits, four characters
    std::array<uint8_t, 4> size;            // 32 bits Synchsafe integer (4 * %0xxxxxxx)
    std::array<uint8_t, 2> flags;           // 16 bits, two flag bytes

    std::string frameIdToStr() const {
        return charsToStr(frame_id);
    }

    // Get the size of the ID3 frame header as int32_t.
    uint32_t getSize() const {
        return fromSynchsafe32(size);
    }
};
#pragma pack(pop)

// The base struct for ID3 frames
struct ID3Frame {
    ID3FrameHeader header{};
    virtual ~ID3Frame() = default;
    virtual void toJson(nlohmann::json& song) const = 0;
};

//
// ID3 Frame types
// From: https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-frames.html#text-information-frames
//

// Text information frames, ID: "T000" - "TZZZ", excluding "TXXX".
struct TextInformationFrame : public ID3Frame {
    uint8_t encoding;           // Text encoding    $xx
    std::string value;          // Information      <text string(s) according to encoding>

    // Constructs a TextInformationFrame, throws on error.
    explicit TextInformationFrame(const ID3FrameHeader frame_header, const std::vector<uint8_t>& data) {
        // Data should never be empty
        if (data.empty()) throw std::runtime_error("TextInformationFrame: empty frame");

        this->header = frame_header;
        encoding = data[0];
        value = parseTextInformationFrame(data, encoding);
    }

    // Append this frame to the JSON.
    void toJson(nlohmann::json& song) const override {
        song[header.frameIdToStr()] = value;
    }

private:
    static std::string parseTextInformationFrame(const std::vector<uint8_t>& data, const uint8_t& text_encoding) {
        // TODO: Needs to account for scenarios where data is corrupted/not according to spec
        std::string result;
        switch (text_encoding) {
            case 0: {
                // ISO-8859-1, terminated with $00 and 1 byte per char.
                const auto it_begin = data.begin() + 1; // Skip first byte, used for encoder
                const auto it_end = std::find(it_begin, data.end(), '\0');
                result = std::string(it_begin, it_end);
                std::cout << "ISO-8859-1\n";
                break;
            }
            case 3: {
                // UTF-8, terminated with $00 and 1 byte per char.
                const auto it_begin = data.begin() + 1; // Skip first byte, used for encoder
                const auto it_end = std::find(it_begin, data.end(), '\0');
                result = std::string(it_begin, it_end);
                std::cout << "UTF-8\n";
                break;
            }
            case 1: {
                // UTF-16, terminated with $00 00 and 2 bytes per char. With byte order mark (BOM)
                // determining endianness. BOM is either FE FE (little endian) or FE FF (big endian).
                // TODO: Add UTF-16 support.
                std::cerr << "UTF-16 text is not yet supported.\n";
                break;
            }
            case 2: {
                // UTF-16BE, terminated with $00 00 and 2 bytes per char.
                // TODO: Add UTF-16BE support.
                std::cerr << "UTF-16BE text is not yet supported.\n";
                break;
            }
            default: {
                std::cerr << "Text encoding was not recognized.\n";
                break;
            }
        }
        return result;
    }
};

// User defined text information frame.
struct TXXX : public ID3Frame {
    uint8_t encoding;           // Text encoding    $xx
    std::string description;    // Description      <text string according to encoding> $00 (00)
    std::string value;          // Value            <text string according to encoding>

    explicit TXXX(const ID3FrameHeader frame_header, const std::vector<uint8_t>& data) {
        // Data should never be empty
        if (data.empty()) throw std::runtime_error("TXXX: empty frame");

        this->header = frame_header;
        encoding = data[0];
        auto [desc, val] = parseTXXXFrame(data, encoding);
        description = std::move(desc);
        value = std::move(val);
    }

    // Append this frame to the JSON.
    void toJson(nlohmann::json& song) const override {
        song["TXXX"][description] = value;
    }

private:
    static std::pair<std::string, std::string> parseTXXXFrame(const std::vector<uint8_t>& data, const uint8_t& text_encoding) {
        std::string desc;
        std::string val;
        switch (text_encoding) {
            case 0:
                // ISO-8859-1, terminated with $00 and 1 byte per char.
            case 3: {
                // UTF-8, terminated with $00 and 1 byte per char.
                auto it_begin = data.begin() + 1; // Skip first byte, used for encoder
                auto it_end = std::find(it_begin, data.end(), '\0');
                desc = std::string(it_begin, it_end);
                it_begin = it_end + 1;
                it_end = std::find(it_begin, data.end(), '\0');
                val = std::string(it_begin, it_end);
                if (text_encoding == 0) std::cout << "ISO-8859-1\n";
                if (text_encoding == 3) std::cout << "UTF-8\n";
                break;
            }
            case 1: {
                // UTF-16, terminated with $00 00 and 2 bytes per char. With byte order mark (BOM)
                // determining endianness. BOM is either FE FE (little endian) or FE FF (big endian).
                // TODO: Add UTF-16 support.
                std::cerr << "UTF-16 text is not yet supported.\n";
                break;
            }
            case 2: {
                // UTF-16BE, terminated with $00 00 and 2 bytes per char.
                // do magic stuff
                // TODO: Add UTF-16BE support.
                std::cerr << "UTF-16BE text is not yet supported.\n";
                break;
            }
            default: {
                std::cerr << "Text encoding was not recognized.\n";
                break;
            }
        }

        return {desc, val};
    }
};

// Comment frame.
struct COMM : public ID3Frame {
    uint8_t encoding;                   // Text encoding          $xx
    std::array<uint8_t, 3> language{};  // Language               $xx xx xx
    std::string description;            // Short content descrip. <text string according to encoding> $00 (00)
    std::string value;                  // The actual text        <full text string according to encoding>

    explicit COMM(const ID3FrameHeader frame_header, const std::vector<uint8_t>& data) {
        // Data should never be empty
        if (data.empty()) throw std::runtime_error("COMM: empty frame");

        this->header = frame_header;
        encoding = data[0];
        language = {data[1], data[2], data[3]};
        auto [desc, val] = parseCOMMFrame(data, encoding);
        description = std::move(desc);
        value = std::move(val);
    }

    // Append this frame to the JSON.
    void toJson(nlohmann::json& song) const override {
        song["COMM"][description] = value;
    }

private:
    static std::pair<std::string, std::string> parseCOMMFrame(const std::vector<uint8_t>& data, const uint8_t& text_encoding) {
        // TODO: Looks identical to parseTXXXFrame(), maybe combine or inherit?
        std::string desc;
        std::string val;
        switch (text_encoding) {
            case 0:
                // ISO-8859-1, terminated with $00 and 1 byte per char.
            case 3: {
                // UTF-8, terminated with $00 and 1 byte per char.
                auto it_begin = data.begin() + 4; // Skip 4bytes, encoder + language
                auto it_end = std::find(it_begin, data.end(), '\0');
                desc = std::string(it_begin, it_end);
                it_begin = it_end + 1;
                it_end = std::find(it_begin, data.end(), '\0');
                val = std::string(it_begin, it_end);
                if (text_encoding == 0) std::cout << "UTF-8\n";
                if (text_encoding == 3) std::cout << "ISO-8859-1\n";
                break;
            }
            case 1: {
                // UTF-16, terminated with $00 00 and 2 bytes per char. With byte order mark (BOM)
                // determining endianness. BOM is either FE FE (little endian) or FE FF (big endian).
                // TODO: Add UTF-16 support.
                std::cerr << "UTF-16 text is not yet supported.\n";
                break;
            }
            case 2: {
                // UTF-16BE, terminated with $00 00 and 2 bytes per char.
                // do magic stuff
                // TODO: Add UTF-16BE support.
                std::cerr << "UTF-16BE text is not yet supported.\n";
                break;
            }
            default: {
                std::cerr << "Text encoding was not recognized.\n";
                break;
            }
        }
        return {desc, val};
    }
};

// Attached picture frame.
struct APIC : public ID3Frame {
    uint8_t encoding;           // Text encoding      $xx
    std::string mime_type;      // MIME type          <text string> $00
    uint8_t picture_type;       // Picture type       $xx
    std::string description;    // Description        <text string according to encoding> $00 (00)
    std::vector<uint8_t> data;  // <binary data>

    explicit APIC(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
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
    void toJson(nlohmann::json& song) const override {
        song["APIC"]["MIME type"] = mime_type;
        song["APIC"]["Picture type"] = picture_type;
        song["APIC"]["Description"] = description;
        song["APIC"]["Data"] = base64Encode(data);
    }

private:
    static std::tuple<std::string, uint8_t, std::string, std::vector<uint8_t>> parseAPICFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
        std::string mime;
        uint8_t picture = 0;
        std::string desc;
        std::vector<uint8_t> picture_data{};
        switch (text_encoding) {
            case 0:
                // ISO-8859-1, terminated with $00 and 1 byte per char.
            case 3: {
                // UTF-8, terminated with $00 and 1 byte per char.
                auto it_begin = frame_data.begin() + 1; // Skip first byte, used for encoder
                auto it_end = std::find(it_begin, frame_data.end(), '\0');
                mime = std::string(it_begin, it_end);
                it_begin = it_end + 1;

                // If there is no more data, end here.
                if (it_begin == frame_data.end()) break;

                picture = *it_begin;
                ++it_begin;

                it_end = std::find(it_begin, frame_data.end(), '\0');
                desc = std::string(it_begin, it_end);

                it_begin = it_end + 1;

                // If there is no more data, end here.
                if (it_begin == frame_data.end()) break;

                it_end = frame_data.end();
                picture_data = std::vector<uint8_t>(it_begin, it_end);

                if (text_encoding == 0) std::cout << "UTF-8\n";
                if (text_encoding == 3) std::cout << "ISO-8859-1\n";
                break;
            }
            case 1: {
                // UTF-16, terminated with $00 00 and 2 bytes per char. With byte order mark (BOM)
                // determining endianness. BOM is either FE FE (little endian) or FE FF (big endian).
                // TODO: Add UTF-16 support.
                std::cerr << "UTF-16 text is not yet supported.\n";
                break;
            }
            case 2: {
                // UTF-16BE, terminated with $00 00 and 2 bytes per char.
                // do magic stuff
                // TODO: Add UTF-16BE support.
                std::cerr << "UTF-16BE text is not yet supported.\n";
                break;
            }
            default: {
                std::cerr << "Text encoding was not recognized.\n";
                break;
            }
        }

        return {mime, picture, desc, picture_data};
    }
};

// // URL link frames, ID: "W000" - "WZZZ", excluding "WXXX".
// struct URLLinkFrame {
//     std::string url;
// };

// // Unsynchronised lyrics/text transcription frames.
// struct USLT {
//     uint8_t encoding;
//     std::array<uint8_t, 3> language;
//     std::string content_descriptor;
//     std::string lyrics;
// };

// TODO: Delete after refactor frame parsing
// struct ID3Frame {
//     ID3FrameHeader header;
//     std::vector<uint8_t> data;  // Data for the frame, see structs defined above.
//
//     // Parse the frame data, according to frame type.
//     std::variant<std::monostate, TextInformationFrame, TXXX, COMM, APIC> parse() const {
//         // TODO: Improve fallback when frame has no data.
//         // Abort when there is no frame data to parse
//         if (header.getSize() <= 0 || data.empty()) {
//             return std::monostate{};
//         }
//         std::string id = header.frameIdToStr();
//         if (id[0] == 'T' && id != "TXXX") return TextInformationFrame(data);
//         if (id == "TXXX") return TXXX(data);
//         if (id == "COMM") return COMM(data);
//         if (id == "APIC") return APIC(data);
//         return std::monostate{};
//     }
// };

ID3Header parseId3Header(std::ifstream& fin, bool verbose = false);
std::map<std::string, std::vector<std::string>> extractId3Frames(std::ifstream& fin, uint32_t id3_size, bool verbose = false);
std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader header, const std::vector<uint8_t>& data);
// TODO: Delete after refactoring parsing:
// std::string readTextFrameData(const ID3Frame &frame);

#endif //MLI_ID3_PARSER_H
