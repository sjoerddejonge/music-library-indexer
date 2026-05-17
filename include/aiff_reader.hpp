//
// Created by Sjoerd de Jonge on 02/04/2026.
//

#ifndef MLI_AIFF_READER_H
#define MLI_AIFF_READER_H
#include <array>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

// TODO: Refactor comments to doxygen style comments

// Audio IFF uses big-endian order, meaning the most significant byte is stored at the smallest memory address


namespace aiff {
/// AIFF comment from the Comments Chunk. Contents of this struct should be included in the JSON.
struct Comment {
    // TODO: Include timestamp in Comment struct
    /// ID of the marker that this comment is linked to, 0 if not linked to a marker.
    std::uint16_t marker_id;
    /// Comment text.
    std::string text;
};

/// Metadata to be extracted from the AIFF file.
struct Metadata {
    /// The name chunk contains the name of the sampled sound.
    std::vector<uint8_t> name;                  // Name chunk
    /// The author chunk contains one or more author names.
    std::vector<uint8_t> auth;                  // Author chunk
    /// Contains a copyright notice for the sound.
    std::vector<uint8_t> copyright;             // Copyright chunk
    /// Annotation chunk contains a comment.
    std::vector<uint8_t> anno;                  // Annotation chunk
    /// Can contain multiple comments.
    std::vector<Comment> comments;              // Comment chunk(s)
    /// The position of the ID3 tag on the ifstream.
    std::optional<std::streampos> id3_pos;      // Position of the ID3 chunk (start of data part)
};

/**
 * @brief Scan an AIFF file for its metadata and ID3 tag position.
 * @param fin The std::ifstream of the AIFF file
 * @param verbose Gives console output if true
 * @return An aiff::Metadata struct containing ID3 tag position @code id3_pos@endcode
 */
Metadata scanFile(std::ifstream& fin, bool verbose = false);
}

#endif //MLI_AIFF_READER_H
