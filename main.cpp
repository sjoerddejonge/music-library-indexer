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
 *  5. Read each frame in ID3 tag, parse to string and add to JSON song
 *  6. Add JSON song to JSON library
 *
 */


#include <iostream>
#include <filesystem>
#include <fstream>
#include "aiff_reader.h"
#include "id3_parser.h"
#include "include/nlohmann/json.hpp"
#include "library_scanner.h"
#include "options.h"

int main() {
    const std::string project_root = PROJECT_ROOT;
    const std::filesystem::path directory_path = project_root + "/music";

    const IndexOptions options = {
        .verbose = false,
        .subdirectories = true,
        .include_apic = false,
        .output_type = Output::FILE,
    };

    // Recursive directory scanning:
    const nlohmann::json library = libraryToJson(directory_path, options);

    // Output in console:
    if (options.output_type == Output::CONSOLE) std::cout << "Output JSON: \n" << library.dump(4) << std::endl;

    // Output as file:
    else if (options.output_type == Output::FILE) {
        // Create a unique filename for directory 'directory_path':
        std::filesystem::path output_path = makeUniqueFilePath(options.output_filename, directory_path);
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

    return 0;
}
