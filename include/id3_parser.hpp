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
#include "options.hpp"
#include "util/base64.hpp"
#include "util/helpers.hpp"
#include "nlohmann/json.hpp"


// ID3v2 also uses big-endian order
// https://id3.org/id3v2.3.0#ID3_tag_version_2.3.0
// https://id3.org/id3v2.4.0-structure

/**
 * ID3 tag consists of
 * 1. A header
 * 2. Multiple frames consisting of:
 *  a. A header
 *  b. Frame data
 */

// TODO: Add errors to frame type constructors and parsing functions to account for malformed data
// TODO: Only call fromSynchsafe32() helper when ID3 header flag bit `a` is not set
    // TODO: Add ID3HeaderOptions struct to store ID3 header flags?
    // > That way we can pass down information like the unsynchronization flag or ID3 version (v2.3.0 or v2.4.0)
// TODO: Move helper functions that are only used in id3_parser.cpp to the implementation file only (declutter global namespace)

static_assert(true); // Dummy declaration to stop clangd bug from giving false warning for #pragma pack(push, 1) line

#pragma pack(push, 1) // Prevent compiler from adding padding bytes.
/// Header for the entire ID3 tag.
struct ID3Header {
    /// Always "ID3" to indicate this is an ID3v2 tag.
    std::array<char, 3> file_identifier;    // 24 bits "ID3"

    /// Version of the ID3v2 tag. The first byte is the major version, the second is its revision number. For example:
    /// '4' and '0' would indicate this an ID3v2.4.0 tag.
    std::array<uint8_t, 2> version;         // 16 bits

    /// Flag byte (%abcd0000). Contains 8 flag bits of which 3 (ID3v2.3.0) or 4 (ID3v2.4.0) are used:
    ///
    /// a. Unsynchronization - Set if unsynchronization is applied on all frames.
    ///
    /// b. Extended header - Set if the header is followed by an extended header.
    ///
    /// c. Experimental indicator - Set when tag is in experimental state.
    ///
    /// d. Footer present (for ID3v2.4.0) - Set when a footer is present at the very end of the tag.
    uint8_t flags;                          // 8 bits, first 3-4 bits are used as flags, others 0 (%abcd0000):

    /// The sum of the byte length of the extended header, the padding and the frames (after unsynchronisation).
    ///
    /// If a footer is present this equals to 'total size' minus 20 bytes, otherwise 'total size' minus the 10 bytes
    /// for this header.
    std::array<uint8_t, 4> size;            // 32 bits (for v2.4.0 Synchsafe) integer (4 * %0xxxxxxx)

    /**
     * @brief Get the size of the ID3 header as int32_t. Converts from big endian to native endian.
     * @return The size of the data in this frame, as an unsigned 32-bit integer
     */
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
/// Header for an ID3 frame.
struct ID3FrameHeader {
    /// ID of the frame, indicating the type. For example: "COMM" for a comment frame.
    std::array<char, 4> frame_id;           // 32 bits, four characters

    /// The size of the data in the final frame, after encryption and compression.
    ///
    /// For ID3v2.4.0 this is a synchsafe integer: a 32-bit integer storing 28 bits of information, where each most
    /// significant bit is always 0.
    std::array<uint8_t, 4> size;            // 32 bits (for v2.4.0 Synchsafe) integer (4 * %0xxxxxxx)

    /// Two flag bytes. The first byte is for 'status messages' and the second byte is a format description.
    ///
    /// If an unknown flag is set in the first byte the frame MUST NOT be changed without that bit cleared. If an
    /// unknown flag is set in the second byte the frame is likely to not be readable.
    ///
    /// See: https://id3.org/id3v2.4.0-structure#:~:text=Frame%20header%20flags,-In for flag bit details.
    std::array<uint8_t, 2> flags;           // 16 bits, two flag bytes

    /**
     * @brief Transform the std::array<char,4> frame_id to a std::string.
     * @return A std::string of the frame ID (4 characters).
     */
    [[nodiscard]] std::string frameIdToStr() const {
        return charsToStr(frame_id);
    }

    /**
     * @brief Get the size of the ID3 frame header as int32_t. Converts from big endian to native endian.
     * @param synchsafe Whether the size is a synchsafe integer (requires additional processing)
     * @return The size of the data in this frame, as an unsigned 32-bit integer
     */
    [[nodiscard]] uint32_t getSize(bool synchsafe) const {
        if (synchsafe) return fromSynchsafe32(size);
        return fromBigEndianInt(fromArrayToInt32(size));
    }
};
#pragma pack(pop)

/// Generic struct for an entire ID3 frame, including header.
struct ID3Frame {
    /// Header of the ID3 frame.
    ID3FrameHeader header{};
    /// Destructor.
    virtual ~ID3Frame() = default;

    /**
     * @brief Virtual function to write ID3 struct to a JSON.
     * @return JSON with the data of a single ID3 frame.
     */
    [[nodiscard]] virtual nlohmann::json toJson() const = 0;

    /**
     * @brief Whether the returned frame is of array type. Array type frames can have multiple occurrences of the same
     * ID3 frame. For example: multiple COMM or TXXX frames in one song.
     */
    virtual bool isArrayType() {
        return false;
    };
};



// ====================================================================================================================
//
//      ID3 Frame types
//      From: https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-frames.html#text-information-frames
//
// ====================================================================================================================



/// Text information frames, ID: "T000" - "TZZZ", excluding "TXXX".
struct TextInformationFrame : public ID3Frame {
    /// Character encoding standard used for the text.
    ///
    /// 0 for ISO-8859-1 \n
    /// 1 for UTF-16 \n
    /// 2 for UTF-16BE \n
    /// 3 for UTF-8
    uint8_t encoding;           // Text encoding    $xx

    /// Information.
    ///
    /// <text string(s) according to encoding>
    std::string value;          // Information      <text string(s) according to encoding>


    //
    /**
     * @brief Constructs a TextInformationFrame struct, throws error on empty frame.
     * @return A TextInformationFrame struct.
     */
    explicit TextInformationFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);

    /**
     * @brief Write this frame to a JSON.
     * @return JSON as {"frame_id": "value"}
     */
    [[nodiscard]] nlohmann::json toJson() const override;

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A string containing the information in TextInformationFrame::value
     */
    static std::string parseTextInformationFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};



/// User defined text information frame, ID: "TXXX".
struct TXXX : public ID3Frame {
    /// Character encoding standard used for the text.
    ///
    /// 0 for ISO-8859-1 \n
    /// 1 for UTF-16 \n
    /// 2 for UTF-16BE \n
    /// 3 for UTF-8
    uint8_t encoding;           // Text encoding    $xx

    /// Description. Terminated with null terminator byte(s) $00 (00).
    ///
    /// <text string according to encoding> $00 (00)
    std::string description;

    /// Value.
    ///
    /// <text string according to encoding>
    std::string value;

    /**
     * @brief Construct a TXXX frame struct, throws error on empty frame.
     * @return A TXXX struct.
     */
    explicit TXXX(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);

    /**
     * @brief Write this frame to a JSON.
     *
     * Because the TXXX is an array type, multiple TXXX frames can be included under the "TXXX" key in the songs' JSON.
     * @return JSON as {"description": "description", "value": "value"}
     */
    [[nodiscard]] nlohmann::json toJson() const override;

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return True.
     */
    bool isArrayType() override;

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A std::pair containing two strings: one for the TXXX::description and one for TXXX::value
     */
    static std::pair<std::string, std::string> parseTXXXFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};



/// Comment frame, ID: "COMM".
struct COMM : public ID3Frame {
    /// Character encoding standard used for the text.
    ///
    /// 0 for ISO-8859-1 \n
    /// 1 for UTF-16 \n
    /// 2 for UTF-16BE \n
    /// 3 for UTF-8
    uint8_t encoding;                   // Text encoding          $xx

    /// Three char bytes for language, for example "eng".
    std::array<char, 3> language{};     // Language               $xx xx xx

    /// Short content description. Terminated with null terminator byte(s) $00 (00).
    ///
    /// <text string according to encoding> $00 (00)
    std::string description;

    /// Value (comment text).
    ///
    /// <full text string according to encoding>
    std::string value;

    /**
     * @brief Construct a COMM frame struct, throws error on empty frame.
     * @return A COMM struct.
     */
    explicit COMM(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);

    /**
     * @brief Write this frame to a JSON.
     *
     * Because the COMM is an array type, multiple COMM frames can be included under the "COMM" key in the songs' JSON.
     * @return JSON as {"comment": "value", "description": "description", "language": "language"}
     */
    [[nodiscard]] nlohmann::json toJson() const override;

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return True.
     */
    bool isArrayType() override;

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A std::pair containing two strings: one for the COMM::description and one for COMM::value
     */
    static std::pair<std::string, std::string> parseCOMMFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};


/// Attached picture frame, ID: "APIC".
struct APIC : public ID3Frame {
    /// Character encoding standard used for the text.
    ///
    /// 0 for ISO-8859-1 \n
    /// 1 for UTF-16 \n
    /// 2 for UTF-16BE \n
    /// 3 for UTF-8
    uint8_t encoding;           // Text encoding      $xx

    /// MIME type, for example: "image/jpeg".
    ///
    /// <text string> $00
    std::string mime_type;      // MIME type

    /// Picture type indicating what the picture represents (for example, album cover).
    ///
    /// See: https://id3.org/id3v2.3.0#:~:text=Picture%20type%3A for all possible types.
    uint8_t picture_type;       // Picture type       $xx

    /// Description. Terminated with null terminator byte(s) $00 (00).
    ///
    /// <text string according to encoding> $00 (00)
    std::string description;    // Description        <text string according to encoding> $00 (00)

    /// The binary pixel data for the image.
    std::vector<uint8_t> data;  // <binary data>

    /**
     * @brief Construct an APIC frame struct, throws error on empty frame.
     * @return An APIC struct.
     */
    explicit APIC(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);

    /**
     * @brief Write this frame to a JSON. The binary data APIC::picture_data will be encoded to text as Base64.
     *
     * Because the APIC is an array type, multiple APIC frames can be included under the "APIC" key in the songs' JSON.
     * @return JSON as {"description": "description", "mime_type": "mime_type", "picture_data": "picture_data,
     * "picture_type": "picture_type"}
     */
    [[nodiscard]] nlohmann::json toJson() const override;

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return True.
     */
    bool isArrayType() override;

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A std::tuple containing a string for APIC::mime_type, a uint8_t for picture_type, a string for APIC::description, and a vector for APIC::picture_data.
     */
    static std::tuple<std::string, uint8_t, std::string, std::vector<uint8_t>> parseAPICFrame(const std::vector<uint8_t>& frame_data, uint8_t text_encoding);
};


/// URL link frames, ID: "W000" - "WZZZ", excluding "WXXX".
struct UrlLinkFrame : public ID3Frame {
    /// The link/URL.
    ///
    /// <text string>
    std::string url;

    /**
     * @brief Construct a UrlLinkFrame frame struct, throws error on empty frame.
     * @return A UrlLinkFrame struct.
     */
    explicit UrlLinkFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);

    /**
     * @brief Write this frame to a JSON.
     * @return JSON as {"frame_id": "url"}
     */
    [[nodiscard]] nlohmann::json toJson() const override;
};


/// User defined link frame, ID: "WXXX".
struct WXXX : public ID3Frame {
    /// Character encoding standard used for the text.
    ///
    /// 0 for ISO-8859-1 \n
    /// 1 for UTF-16 \n
    /// 2 for UTF-16BE \n
    /// 3 for UTF-8
    uint8_t encoding;

    /// Description. Terminated with null terminator byte(s) $00 (00).
    ///
    /// <text string according to encoding> $00 (00)
    std::string description;

    /// The link/URL. \n
    /// Note: WXXX::encoding does not apply to this string!
    ///
    /// <text string>
    std::string url;

    /**
     * @brief Construct a WXXX frame struct, throws error on empty frame.
     * @return A WXXX struct.
     */
    explicit WXXX(ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data);

    /**
     * @brief Write this frame to a JSON.
     *
     * Because the WXXX is an array type, multiple WXXX frames can be included under the "WXXX" key in the songs' JSON.
     * @return JSON as {"description": "description", "url": "url"}
     */
    [[nodiscard]] nlohmann::json toJson() const override;

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return True.
     */
    bool isArrayType() override;
};

// TODO: Add support for USLT frames:
// // Unsynchronised lyrics/text transcription frames.
// struct USLT {
//     uint8_t encoding;
//     std::array<uint8_t, 3> language;
//     std::string content_descriptor;
//     std::string lyrics;
// };



// ====================================================================================================================
//
//      Public Functions
//
// ====================================================================================================================

/**
 * @brief Extract all information from ID3 tag and write to JSON.
 * @param fin Reference to the ifstream of the music file
 * @param id3_pos The std::streampos of the ID3 tag in the music file
 * @param options A struct with options for running the command. For default see include/options.hpp
 * @return JSON with all parsed ID3 tag information of a song
 */
nlohmann::json id3ToJson(std::ifstream& fin, const std::streampos &id3_pos, const IndexOptions& options);



// ====================================================================================================================
//
//      Internal Template Functions
//
// ====================================================================================================================

/**
 * @brief Find terminating double byte $00 00 in a vector of bytes.
 * @tparam Iterator A random access iterator.
 * @param begin Start Iterator of the vector.
 * @param end End Iterator of the vector
 * @return Either an Iterator at the null terminating byte or an Iterator at the end of the vector
 */
template <std::random_access_iterator Iterator>
static Iterator findTerminatingIterator(Iterator begin, Iterator end) {
    Iterator it = begin;
    while (it < end) {
        auto next = std::next(it, 1);
        if (next == end) break;
        if (*it == 0x00 && *next == 0x00) return it;

        std::advance(it, 2);
    }
    return end;
}

/**
 * @brief Reads a field of a frame to UTF-8 string. From the start to the end OR null terminator if found.
 * @param begin Start iterator of a data vector
 * @param end_of_vector End iterator of data vector
 * @param encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
 * @param little_endian Default = false. In case of UTF-16 with no BOM (which occurs only at first field), set this to true when
 * byte pairs are little endian
 * @return Tuple of [field_string, iterator_past_terminator, bool_little_endian].
*/
template <std::input_iterator Iterator>
static std::tuple<std::string, Iterator, std::optional<bool>> readFieldToUtf8(Iterator begin, Iterator end_of_vector, int encoding, bool little_endian = false) {
    bool double_byte = (encoding == 1 || encoding == 2); // True for UTF-16 encodings which uses a byte pair per character
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

#endif //MLI_ID3_PARSER_H
