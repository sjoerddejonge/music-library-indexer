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


// ID3v2 also uses big-endian order
// https://id3.org/id3v2.3.0#ID3_tag_version_2.3.0
// https://id3.org/id3v2.4.0-structure

// Header for the ID3 tag. Contains the version, flags, and size of the tag. Should be 10 bytes / 80 bits.
#pragma pack(push, 1) // Prevent compiler from adding padding bytes.
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
struct ID3FrameHeader {
    std::array<char, 4> frame_id;           // 32 bits, four characters
    std::array<uint8_t, 4> size;            // 32 bits Synchsafe integer (4 * %0xxxxxxx)
    std::array<uint8_t, 2> flags;           // 16 bits, two flag bytes
};
#pragma pack(pop)

struct ID3Frame {
    ID3FrameHeader header;
    std::vector<uint8_t> data;  // Data for the frame
};

//
// ID3 Frame types
// From: https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-frames.html#text-information-frames
//

// Text information frames, ID: "T000" - "TZZZ", excluding "TXXX".
struct TextInformationFrame {
    uint8_t encoding;
    std::string information;
};

// User defined text information frame.
struct TXXX {
    uint8_t encoding;
    std::string description;
    std::string value;
};

// Comment frame.
struct COMM {
    uint8_t encoding;
    std::array<uint8_t, 3> language;
    std::string content_descriptor;
    std::string text;
};

// Attached picture frame.
struct APIC {
    uint8_t encoding;
    std::string mime_type;
    uint8_t picture_type;
    std::string description;
    std::vector<uint8_t> data;
};

// // URL link frames, ID: "W000" - "WZZZ", excluding "WXXX".
// struct URLLinkFrame {
//     std::string url;
// };

// Unsynchronised lyrics/text transcription frames.
// struct USLT {
//     uint8_t encoding;
//     std::array<uint8_t, 3> language;
//     std::string content_descriptor;
//     std::string lyrics;
// };

ID3Header parseId3Header(std::ifstream& fin, bool verbose = false);
std::map<std::string, std::vector<std::string>> extractId3Frames(std::ifstream& fin, uint32_t tag_size, bool verbose = false);
std::string readTextFrameData(const ID3Frame &frame);

#endif //MLI_ID3_PARSER_H
