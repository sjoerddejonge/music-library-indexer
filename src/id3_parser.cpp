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

// Anonymous namespace hiding internal functions:
namespace {
static_assert(true); // Dummy declaration to stop clangd bug from giving false warning for #pragma pack(push, 1) line

//
//      Header structs
//

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
     * @brief Get the size of the ID3 header as uint32_t. Converts from big endian to native endian.
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
    /// @see: https://id3.org/id3v2.4.0-structure#:~:text=Frame%20header%20flags,-In
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
     * @return @code false@endcode or @code true@endcode
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
    explicit TextInformationFrame(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
        // Data should never empty
        if (frame_data.empty()) throw std::runtime_error("TextInformationFrame: empty frame of type " + frame_header.frameIdToStr());
        header = frame_header;
        encoding = frame_data[0];
        value = parseTextInformationFrame(frame_data, encoding);
    }

    /**
     * @brief Write this frame to a JSON.
     * @return JSON as @code {"frame_id": "value"}@endcode
     */
    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json frame;
        if (!value.empty()) frame[header.frameIdToStr()] = value;
        return frame;
    }

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A string containing the information in TextInformationFrame::value
     */
    static std::string parseTextInformationFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
        // TODO: Needs to account for scenarios where data is corrupted/not according to spec
        // Determine the start iterator as start of value field
        const auto it_start = std::next(frame_data.begin(), 1); // Skip 1 byte (encoding)
        auto [val, it_after_desc, little_endian] =
            readFieldToUtf8(it_start, frame_data.end(), text_encoding);
        return val;
    }
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
    explicit TXXX(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
        // Data should never be empty
        if (frame_data.empty()) throw std::runtime_error("TXXX: empty frame");
        header = frame_header;
        encoding = frame_data[0];
        auto [desc, val] = parseTXXXFrame(frame_data, encoding);
        description = std::move(desc);
        value = std::move(val);
    }

    /**
     * @brief Write this frame to a JSON.
     *
     * Because the TXXX is an array type, multiple TXXX frames can be included under the "TXXX" key in the songs' JSON.
     * @return JSON as @code {"description": "description", "value": "value"}@endcode
     */
    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json frame;
        if (!description.empty() || !value.empty()) {
            frame["description"] = description;
            frame["value"] = value;
        }
        return frame;
    }

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return True.
     */
    bool isArrayType() override {
        return true;
    }

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A std::pair containing two strings: one for the TXXX::description and one for TXXX::value
     */
    static std::pair<std::string, std::string> parseTXXXFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
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
    explicit COMM(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
        // Data should never be empty
        if (frame_data.empty()) throw std::runtime_error("COMM: empty frame");
        this->header = frame_header;
        encoding = frame_data[0];
        language = {static_cast<char>(frame_data[1]), static_cast<char>(frame_data[2]), static_cast<char>(frame_data[3])};
        auto [desc, val] = parseCOMMFrame(frame_data, encoding);
        description = std::move(desc);
        value = std::move(val);
    }

    /**
     * @brief Write this frame to a JSON.
     *
     * Because the COMM is an array type, multiple COMM frames can be included under the "COMM" key in the songs' JSON.
     * @return JSON as @code {"comment": "value", "description": "description", "language": "language"}@endcode
     */
    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json frame;
        if (!description.empty() || !value.empty()) {
            frame["language"] = charsToStr(language);
            frame["description"] = description;
            frame["comment"] = value;
        }
        return frame;
    }

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return @code true@endcode
     */
    bool isArrayType() override {
        return true;
    }

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A std::pair containing two strings: one for the COMM::description and one for COMM::value
     */
    static std::pair<std::string, std::string> parseCOMMFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
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
    /// @see https://id3.org/id3v2.3.0#:~:text=Picture%20type%3A
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

    /**
     * @brief Write this frame to a JSON. The binary data APIC::picture_data will be encoded to text as Base64.
     *
     * Because the APIC is an array type, multiple APIC frames can be included under the "APIC" key in the songs' JSON.
     * @return JSON as @code {"description": "description", "mime_type": "mime_type", "picture_data": "picture_data,
     * "picture_type": "picture_type"} @endcode
     */
    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json frame;
        if (!data.empty()) {
            frame["mime_type"] = mime_type;
            frame["picture_type"] = picture_type;
            frame["description"] = description;
            frame["picture_data"] = base64Encode(data);
        }
        return frame;
    }

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return @code true@endcode
     */
    bool isArrayType() override {
        return true;
    }

private:
    /**
     * @brief (Private) Parse frame data into struct member variable(s).
     * @param frame_data Vector containing byte data for the entire frame minus the header
     * @param text_encoding [0] ISO-8859-1, [1] UTF-16, [2] UTF-16BE, [3] UTF-8
     * @return A std::tuple containing a string for APIC::mime_type, a uint8_t for picture_type, a string for APIC::description, and a vector for APIC::picture_data.
     */
    static std::tuple<std::string, uint8_t, std::string, std::vector<uint8_t>> parseAPICFrame(const std::vector<uint8_t>& frame_data, const uint8_t text_encoding) {
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
    explicit UrlLinkFrame(const ID3FrameHeader frame_header, const std::vector<uint8_t> &frame_data) {
        // Data should never be empty
        if (frame_data.empty()) throw std::runtime_error("UrlLinkFrame: empty frame");

        header = frame_header;
        auto [text, iter, opt] = readFieldToUtf8(frame_data.begin(), frame_data.end(), 0);
        url = text;
    }

    /**
     * @brief Write this frame to a JSON.
     * @return JSON as @code {"frame_id": "url"}@endcode
     */
    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json frame;
        if (!url.empty()) {
            frame[header.frameIdToStr()] = url;
        }
        return frame;
    }
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
    explicit WXXX(const ID3FrameHeader frame_header, const std::vector<uint8_t>& frame_data) {
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

    /**
     * @brief Write this frame to a JSON.
     *
     * Because the WXXX is an array type, multiple WXXX frames can be included under the "WXXX" key in the songs' JSON.
     * @return JSON as @code {"description": "description", "url": "url"}@endcode
     */
    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json frame;
        if (!url.empty()) {
            frame["description"] = description;
            frame["url"] = url;
        }
        return frame;
    }

    /**
     * @brief Whether this type of ID3 frame can have multiple instances in one song.
     * @return @code true@endcode
     */
    bool isArrayType() override {
        return true;
    }
};

// TODO: Add support for USLT frames:
// // Unsynchronised lyrics/text transcription frames.
// struct USLT {
//     uint8_t encoding;
//     std::array<uint8_t, 3> language;
//     std::string content_descriptor;
//     std::string lyrics;
// };



//
//      Internal implementation functions:
//

// Forward declarations
static ID3Header parseId3Header(std::ifstream& fin, const IndexOptions& options);
static nlohmann::json extractId3Frames(std::ifstream& fin, ID3Header id3_header, const IndexOptions& options);
static std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& data, const IndexOptions& options);

/**
 * @brief Parses the header of an ID3 tag.
 * @param fin Const reference to a std::ifstream at the start of the ID3 tag
 * @param options IndexOptions of which the option 'verbose' is used to enable/disable console output
 * @return Struct ID3Header containing the file_identifier, version, flags, and size
 */
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

/**
 * @brief Scans all supported ID3 frames of a song and writes them to the JSON.
 * @param fin Const reference to a std::ifstream
 * @param id3_header Header of the ID3 tag
 * @param options IndexOptions for 'verbose' and 'include_apic'
 * @return JSON that holds ID3 frame data of a song
 */
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

/**
 * @brief Create an ID3 frame struct for the supported ID3 frames.
 * @param frame_header Header of the ID3 frame
 * @param data Byte vector with the data of the frame minus the header
 * @param options IndexOptions for 'include_apic' to toggle APIC frame inclusion in output
 * @return A std::unique_ptr to the ID3 frame struct
 */
static std::unique_ptr<ID3Frame> makeFrame(ID3FrameHeader frame_header, const std::vector<uint8_t>& data, const IndexOptions& options) {
    const std::string id = frame_header.frameIdToStr();
    if (id == "TXXX") return std::make_unique<TXXX>(frame_header, data);
    if (id == "WXXX") return std::make_unique<WXXX>(frame_header, data);
    if (id == "COMM") return std::make_unique<COMM>(frame_header, data);
    if (id == "APIC" && options.include_apic) return std::make_unique<APIC>(frame_header, data);
    if (id[0] == 'T') return std::make_unique<TextInformationFrame>(frame_header, data);
    if (id[0] == 'W') return std::make_unique<UrlLinkFrame>(frame_header, data);
    return nullptr;
}
} // namespace

//
//      Public interface functions:
//

nlohmann::json id3ToJson(std::ifstream& fin, const std::streampos &id3_pos, const IndexOptions& options) {
    // Move to start of ID3 tag.
    fin.seekg(id3_pos);
    // Extract ID3 header.
    const ID3Header id3_header = parseId3Header(fin, options);
    // Extract ID3 frames and add to JSON.
    nlohmann::json song = extractId3Frames(fin, id3_header, options);
    // "ID3" file identifier in the header guarantees this is ID3v2.X
    song["id3_version"] = std::format("2.{}.{}", id3_header.version[0], id3_header.version[1]);
    return song;
}
