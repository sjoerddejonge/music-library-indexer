//
// Created by Sjoerd de Jonge on 24/04/2026.
//

#include "../include/commands.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "library_scanner.hpp"
#include "nlohmann/json.hpp"
#include "util/helpers.hpp"
#include "program_info.hpp"

// Scan the directory for compatible files, extract ID3 tags for each file, then append to JSON.
// The JSON is either printed to console, written as file, or discarded.
//
// Arguments:
// directory_path       Path to the directory to scan.
// options              IndexOptions to configure options for this command, see include/options.hpp
void commands::index(const std::filesystem::path& directory_path, const IndexOptions& options) {
    // Directory scanning:
    const nlohmann::json library = libraryToJson(directory_path, options);

    // Output in console:
    if (options.output_type == Output::CONSOLE) std::cout << "Output JSON: \n" << library.dump(4) << std::endl;

    // Output as file:
    else if (options.output_type == Output::FILE) {
        // Create a unique filename for directory 'directory_path':
        std::filesystem::path output_path = makeUniqueFilePath(options.output_filename, directory_path);
        if (library["songs"].empty()) {
            std::cout << "No compatible files found.\n";
            return;
        }
        std::cout << "Read " << library["songs"].size() << " compatible files.\n";
        std::cout << "File written to: " + output_path.string() << std::endl;
        std::ofstream outfile(output_path, std::ios::out);
        if (outfile.is_open()) {
            outfile << library.dump(4) << std::endl;
        }
        if (!outfile.good()) {
            std::cerr << "Failed to write to file: " << output_path.string() << std::endl;
        }
        outfile.close();
    }
}

// Print help text for instructions on how to use this program.
void commands::help() {
                //  ================================================================================
    std::cout   << "Music Library Indexer" << std::format("                     version: {}\n", program::version())
                << "\n"
                << std::format("  usage: {} <command> [<args>]\n", program::name())
                << "\n"
                << "Supported commands:\n"
                << "index       Runs the Music Library Indexer in the current directory. Scans all\n"
                << "            (sub)directories for .aiff files, extracts id3 tag metadata, and\n"
                << "            writes to a JSON in the current/target directory.\n"
                << "            args:\n"
                << "            [<path>]    Full path to a directory to scan\n"
                << "            [-a]        Include APIC (attached picture) frame in JSON, with \n"
                << "                        base64 encoding\n"
                << "            [-v]        Verbose console output\n"
                << "            [--shallow] Single directory only (no subdirectories)\n"
                << "\n"
                << "help        Prints this text.\n"
    ;           //  ================================================================================
}
