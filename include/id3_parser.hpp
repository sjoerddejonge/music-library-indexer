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
#include "options.hpp"
#include "util/base64.hpp"
#include "util/helpers.hpp"
#include "nlohmann/json.hpp"


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
    std::array<uint8_t, 4> size;            // 32 bits (for v2.4.0 Synchsafe) integer (4 * %0xxxxxxx)

    // Get the size of the ID3 header as int32_t. Converts from big endian to native endian.
    [[nodiscard]] uint32_t getSize() const {
        if (version[0] >= 4) {
            return fromSynchsafe32(size);
        }
        // Return array of four uint8_t as one uint32_t
        return fromBigEndianInt(fromArrayToInt32(size));
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
// The header of the ID3 Frame.
struct ID3FrameHeader {
    std::array<char, 4> frame_id;           // 32 bits, four characters
    std::array<uint8_t, 4> size;            // 32 bits (for v2.4.0 Synchsafe) integer (4 * %0xxxxxxx)
    std::array<uint8_t, 2> flags;           // 16 bits, two flag bytes

    [[nodiscard]] std::string frameIdToStr() const {
        return charsToStr(frame_id);
    }

    // Get the size of the ID3 frame header as int32_t. Converts from big endian to native endian.
    [[nodiscard]] uint32_t getSize(const bool synchsafe) const {
        if (synchsafe) return fromSynchsafe32(size);
        return fromBigEndianInt(fromArrayToInt32(size));
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
    // Constructs a TextInformationFrame, throws on error:
    explicit TextInformationFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);
    // Append this frame to the JSON:
    void toJson(nlohmann::json& song) const override;

private:
    // (Private) Parse frame data into struct member variable(s).
    static std::string parseTextInformationFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};

// User defined text information frame.
struct TXXX : public ID3Frame {
    uint8_t encoding;           // Text encoding    $xx
    std::string description;    // Description      <text string according to encoding> $00 (00)
    std::string value;          // Value            <text string according to encoding>
    // Constructs a TXXX, throws on error.
    explicit TXXX(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);
    // Append this frame to the JSON.
    void toJson(nlohmann::json& song) const override;

private:
    // (Private) Parse frame data into struct member variable(s).
    static std::pair<std::string, std::string> parseTXXXFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};

// Comment frame.
struct COMM : public ID3Frame {
    uint8_t encoding;                   // Text encoding          $xx
    std::array<uint8_t, 3> language{};  // Language               $xx xx xx
    std::string description;            // Short content descrip. <text string according to encoding> $00 (00)
    std::string value;                  // The actual text        <full text string according to encoding>
    // Constructs a COMM, throws on error:
    explicit COMM(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);
    // Append this frame to the JSON.
    void toJson(nlohmann::json& song) const override;

private:
    // (Private) Parse frame data into struct member variable(s).
    static std::pair<std::string, std::string> parseCOMMFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};

// Attached picture frame.
struct APIC : public ID3Frame {
    uint8_t encoding;           // Text encoding      $xx
    std::string mime_type;      // MIME type          <text string> $00
    uint8_t picture_type;       // Picture type       $xx
    std::string description;    // Description        <text string according to encoding> $00 (00)
    std::vector<uint8_t> data;  // <binary data>
    // Constructs an APIC, throws on error.
    explicit APIC(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);
    // Append this frame to the JSON.
    void toJson(nlohmann::json& song) const override;

private:
    // (Private) Parse frame data into struct member variable(s).
    static std::tuple<std::string, uint8_t, std::string, std::vector<uint8_t>> parseAPICFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};

// TODO: Add support for URL and USLT frames:
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

// Helper functions:

// Reads a field from its start to null terminator or end, returns {field_string, iterator_past_terminator}.
// The returned field_string is always in UTF-8.
template <std::input_iterator Iterator>
std::tuple<std::string, Iterator, std::optional<bool>> readFieldToUtf8(Iterator begin, Iterator end_of_vector, bool double_byte, int encoding, bool little_endian = false) {
    // UTF-16 case: check for BOM (2 bytes) for endianness and skip BOM.
    if (encoding == 1) {
        if (std::next(begin, 1) != end_of_vector) {
            uint16_t possible_bom = (*begin << 8) | *std::next(begin, 1);
            if (possible_bom == 0xFFFE || possible_bom == 0xFEFF) {
                little_endian = (possible_bom == 0xFFFE);
                std::advance(begin, 2);
            }
        }
    }
    auto field_end = (double_byte)
                      ? findTerminatingIterator(begin, end_of_vector)
                      : std::find(begin, end_of_vector, 0x00);
    std::string desc = toUtf8(begin, field_end, encoding, little_endian);
    begin = field_end;
    if ((double_byte && std::next(begin, 2) >= end_of_vector)
        || (!double_byte && std::next(begin, 1) >= end_of_vector))
    {
        // End of vector, return Iterator end
        if (encoding == 1) return {desc, end_of_vector, little_endian}; // Only return endianness in case of UTF-16.
        return {desc, end_of_vector, std::nullopt};
    }
    std::advance(begin, (double_byte)? 2 : 1);
    if (encoding == 1) return {desc, begin, little_endian}; // Only return endianness in case of UTF-16.
    return {desc, begin, std::nullopt};
};

ID3Header parseId3Header(std::ifstream& fin, const IndexOptions& options);
void extractId3Frames(std::ifstream& fin, ID3Header id3_header, nlohmann::json& song, const IndexOptions& options);
std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader header, const std::vector<uint8_t>& data, const IndexOptions& options);
nlohmann::json id3ToJson(std::ifstream& fin, const IndexOptions& options);

#endif //MLI_ID3_PARSER_H
