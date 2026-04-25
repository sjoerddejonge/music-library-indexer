//
// Created by Sjoerd de Jonge on 20/04/2026.
//

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "library_scanner.hpp"
#include "nlohmann/json.hpp"
#include "aiff_reader.hpp"
#include "id3_parser.hpp"
#include "program_info.hpp"

// Scans all supported music files in the directory (and optionally subdirectories) and writes their ID3 tag data to a JSON.
//
// Arguments:
// - directory_path: The path to the directory to scan for music files.
// - options: A struct with options for running the command. For default see include/options.hpp
nlohmann::json libraryToJson(const std::filesystem::path& directory_path, const IndexOptions& options) {
    // Verify directory_path after adding user input of directory_path.
    // This is a redundant safety check.
    if (!std::filesystem::is_directory(directory_path)) {
        std::cerr << "Directory \"" << directory_path << "\" does not exist.\n";
        throw std::runtime_error("Directory does not exist.");
    }

    // Get current date-time stamp down to seconds
    std::chrono::sys_time<std::chrono::seconds> now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

    // Initialize JSON
    nlohmann::json library;
    library["meta"]["directory"] = directory_path.string();
    library["meta"]["exported_at"] = std::format("{:%Y-%m-%dT%H:%M:%SZ}",now);
    // The tool name is hard-coded as "mli". Here it should NOT use the runtime name ('program::name()').
    library["meta"]["tool"] = PROJECT_NAME;
    library["meta"]["version"] = program::version();
    library["songs"] = nlohmann::json::array(); // Initialize library["songs"] as array

    std::cout << "Reading files in: \"" << directory_path << "\"\n";

    // TODO: Add check for permission to access directory
    // Lambda function for scanning the directories (recursive or not):
    auto scan = [&](const auto& iterator) {
        for (auto const& dir_entry : iterator) {
            if (dir_entry.is_directory() && options.subdirectories) {
                std::cout << "Reading files in: " << dir_entry.path() << "\n";
            }

            /*
             * Add newly supported file formats here:
             */

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
                    if (!song.is_null()) library["songs"].push_back(song);
                }
                catch (const std::exception& e) {
                    std::cerr << "Error occurred: " << e.what() << "\n";
                }
                fin.close();
            }
        }
    };

    if (options.subdirectories) { // Recursive scanning
        const std::filesystem::recursive_directory_iterator iterator{directory_path};
        scan(iterator);
    } else { // Non-recursive scanning
        const std::filesystem::directory_iterator iterator{directory_path};
        scan(iterator);
    }

    library["meta"]["file_count"] = library["songs"].size();

    return library;
}
