/**
 *  Music Library Indexer
 *  @author Sjoerd de Jonge
 *  2026-03-28
 *
 *  Method:
 *  1. Read path to a directory [main.cpp]
 *  2. Loop through all files in directory [library_scanner.cpp]
 *  3. Open ifstream for each file with a supported format (.aiff) [aiff_reader.cpp]
 *  4. Read ifstream to start of ID3 tag [aiff_reader.cpp]
 *  5. Read each frame in ID3 tag, parse to string and add to JSON song [id3_parser.cpp]
 *  6. Add JSON song to JSON library [library_scanner.cpp]
 *  7. Write JSON library to (console/file) [main.cpp]
 *
 */


#include <iostream>
#include <filesystem>
#include <fstream>
#include <format>
#include "aiff_reader.hpp"
#include "commands.hpp"
#include "id3_parser.hpp"
#include "nlohmann/json.hpp"
#include "options.hpp"
#include "program_info.hpp"

// TODO: Add versioning and implement in program_info
// TODO: Add MLI metadata to JSON
// TODO: Fix to-dos in UTF-16 converter for edge cases
// TODO: Tests:
//      TODO: Add UTF-16 surrogate characters to tags of example songs in /music
//      TODO: Add corrupted tags to example songs in /music for testing

// argc is always at least 1
int main(const int argc, char *argv[]) {
    program::init(argc, argv);  // Initialize program info

    #ifdef DEBUG
        /* DEVELOP */
        /* Use this directory_path when running app from the project: */
        std::filesystem::path directory_path = std::filesystem::path(PROJECT_ROOT) / "music";
    #else
        /* RELEASE BUILD */
        /* Use this directory_path when building the project for release: */
        std::filesystem::path directory_path = std::filesystem::current_path();
    #endif

    // Parsing input arguments
    if (argc > 1) {
        // Parse the command to run
        if (strcmp(argv[1], "index") == 0) {
            // Index command
            IndexOptions options = {
                .verbose = false,
                .subdirectories = true,
                .include_apic = false,
                .output_type = Output::FILE,
            };
            // Parse args
            for (int i = 2; i < argc; i++) {
                if (strcmp(argv[i], "-a") == 0) {
                    options.include_apic = true;
                }
                else if (strcmp(argv[i], "-v" ) == 0 || strcmp(argv[i], "--verbose") == 0) {
                    options.verbose = true;
                }
                else if (strcmp(argv[i], "--shallow") == 0) {
                    options.subdirectories = false;
                }
                else {
                    std::filesystem::path p = argv[i];
                    // <PATH> input argument
                    if (std::filesystem::is_directory(p)) {
                        directory_path = p;
                    }
                    else {
                        std::cerr << std::format("Error: unknown arg '{}' for command '{}'. See '{} help'.\n", argv[i], argv[0], program::name());
                        return 1;
                    }
                }
            }
            commands::index(directory_path, options);
        }
        else if (strcmp(argv[1], "help") == 0) {
            // Help command
            commands::help();
        }
        else {
            // Unknown command
            std::cerr << std::format("{}: '{}' is not a valid {} command. See '{} help'.\n",
                    program::name(), argv[1], program::name(), program::name());
            return 1;
        }
    } else {
        commands::help();
    }

    return 0;
}
