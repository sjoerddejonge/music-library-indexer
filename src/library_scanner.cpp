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
    library["meta"]["tool"] = PROJECT_NAME; // Leave this 'hard-coded' to project_name!
    library["meta"]["version"] = program::version();
    library["songs"] = nlohmann::json::array(); // Initialize library["songs"] as array

    std::cout << "Reading files in: " << directory_path << "\n";

    // TODO: Add console output for directories that are skipped due to missing permissions
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
                    aiffData aiff_data = scanFile(fin); // Scan file for id3 position and metadata.
                    if (options.verbose) {
                        std::cout << "~ Filename: " << dir_entry.path() << "\n";
                    }
                    if (aiff_data.id3_pos == std::nullopt) {
                        std::cout << "No ID3 tag found in file.\n";
                    }
                    else {
                        // Returns a JSON with at highest level "id3_version" and "id3_frames":
                        nlohmann::json song = id3ToJson(fin, aiff_data.id3_pos.value(), options);
                        song["filename"] = dir_entry.path().filename().string(); // Add filename to the JSON
                        // Add relative path (excluding directory_path) to JSON:
                        song["relative_path"] = dir_entry.path().lexically_relative(directory_path).string();
                        // Add aiff metadata if present
                        if (!aiff_data.name.empty()) song["aiff_data"]["name"] = iso88591ToUtf8(aiff_data.name.begin(), aiff_data.name.end());
                        if (!aiff_data.auth.empty()) song["aiff_data"]["author"] = iso88591ToUtf8(aiff_data.auth.begin(), aiff_data.auth.end());
                        if (!aiff_data.copyright.empty()) song["aiff_data"]["copyright"] = iso88591ToUtf8(aiff_data.copyright.begin(), aiff_data.copyright.end());
                        if (!aiff_data.anno.empty()) song["aiff_data"]["annotation"] = iso88591ToUtf8(aiff_data.anno.begin(), aiff_data.anno.end());
                        if (!aiff_data.comments.empty()) {
                            for (auto const& comment : aiff_data.comments) {
                                nlohmann::json json_comment;
                                json_comment["text"] = comment.text;
                                json_comment["marker_id"] = comment.marker_id;
                                song["aiff_data"]["comments"].push_back(json_comment);
                            }
                        }
                        if (!song.is_null()) library["songs"].push_back(song);
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error occurred: " << e.what() << "\n";
                }
                fin.close();
            }
        }
    };

    if (options.subdirectories) { // Recursive scanning
        const std::filesystem::recursive_directory_iterator iterator{directory_path, std::filesystem::directory_options::skip_permission_denied};
        scan(iterator);
    } else { // Non-recursive scanning
        const std::filesystem::directory_iterator iterator{directory_path};
        scan(iterator);
    }

    library["meta"]["file_count"] = library["songs"].size();

    return library;
}
