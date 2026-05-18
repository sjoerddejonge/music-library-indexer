//
// Created by Sjoerd de Jonge on 20/04/2026.
//

#ifndef MLI_LIBRARY_SCANNER_H
#define MLI_LIBRARY_SCANNER_H
#include <string>

#include "options.hpp"
#include "nlohmann/json.hpp"

/**
 * @brief Scans all supported music files in the directory (and optionally subdirectories) and writes their ID3 tag data to a JSON.
 * @param directory_path The path to the directory to scan for music files
 * @param options A struct with options for running the command. For default see include/options.hpp
 * @return A JSON with metadata (ID3 tags and format specific) of all songs in supported file formats in the directory
 * @see https://sjoerddejonge.github.io/music-library-indexer/json_output_format.html#example-snapshot
 */
nlohmann::json libraryToJson(const std::filesystem::path& directory_path, const IndexOptions& options);

#endif //MLI_LIBRARY_SCANNER_H
