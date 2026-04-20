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
// - sub_directories: A bool whether to also include subdirectories, default = true.
nlohmann::json libraryToJson(const std::string& directory_path, const bool sub_directories) {
    // TODO: Verify directory_path

    // Create a path:
    const auto path = std::filesystem::path(directory_path);

    nlohmann::json library = nlohmann::json::array();

    // Lambda function for scanning the directories (recursive or not):
    auto scan = [&](const auto& iterator) {
        for (auto const& dir_entry : iterator) {
            // AIFF files
            if (dir_entry.path().extension() == ".aiff" || dir_entry.path().extension() == ".aif") {
                std::ifstream fin{ dir_entry.path(), std::ios_base::binary }; // Create an if-stream to open the file.
                if (!fin) {
                    std::cerr << "Failed to open: " << dir_entry.path() << "\n";
                    continue;
                }
                try {
                    locateId3(fin); // Skip ifstream to the start of the ID3 tag.
                    const nlohmann::json song = id3ToJson(fin);
                    library.push_back(song);
                }
                catch (const std::exception& e) {
                    std::cerr << "Error occurred: " << e.what() << "\n";
                }
            }
        }
    };

    if (sub_directories) { // Recursive scanning
        const std::filesystem::recursive_directory_iterator iterator{path};
        scan(iterator);
    } else { // Non-recursive scanning
        const std::filesystem::directory_iterator iterator{path};
        scan(iterator);
    }

    return library;
}
