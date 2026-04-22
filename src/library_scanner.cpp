//
// Created by Sjoerd de Jonge on 20/04/2026.
//

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "library_scanner.h"
#include "util/json.hpp"
#include "aiff_reader.h"
#include "id3_parser.h"

// Scans all supported music files in the directory (and optionally subdirectories) and writes their ID3 tag data to a JSON.
//
// Arguments:
// - directory_path: The path to the directory to scan for music files.
// - options: A struct with options for running the command. For default see include/options.h
nlohmann::json libraryToJson(const std::string& directory_path, const IndexOptions& options) {
    // TODO: Verify directory_path (but it is unnecessary if user only calls from inside a directory)

    // Create a path:
    const auto path = std::filesystem::path(directory_path);

    nlohmann::json library = nlohmann::json::array();

    std::cout << "Reading files in: \"" << directory_path << "\"\n";

    // Lambda function for scanning the directories (recursive or not):
    auto scan = [&](const auto& iterator) {
        for (auto const& dir_entry : iterator) {
            if (dir_entry.is_directory()) {
                std::cout << "Reading files in: " << dir_entry.path() << "\n";
            }
            // AIFF files
            if (dir_entry.path().extension() == ".aiff" || dir_entry.path().extension() == ".aif") {
                std::ifstream fin{ dir_entry.path(), std::ios_base::binary }; // Create an if-stream to open the file.
                if (!fin) {
                    std::cerr << "Failed to open: " << dir_entry.path() << "\n";
                    continue;
                }
                try {
                    locateId3(fin); // Skip ifstream to the start of the ID3 tag.
                    const nlohmann::json song = id3ToJson(fin, options);
                    if (!song.is_null()) library.push_back(song);
                }
                catch (const std::exception& e) {
                    std::cerr << "Error occurred: " << e.what() << "\n";
                }
                fin.close();
            }
        }
    };

    if (options.subdirectories) { // Recursive scanning
        const std::filesystem::recursive_directory_iterator iterator{path};
        scan(iterator);
    } else { // Non-recursive scanning
        const std::filesystem::directory_iterator iterator{path};
        scan(iterator);
    }

    return library;
}
