/**
 *  Music Library Indexer
 *  @author Sjoerd de Jonge
 *  2026-03-28
 *
 *  Method:
 *  1. Create empty JSON.
 *  2. Loop through all files in current directory.
 *      2a. For each .aiff file:
 *          Add new object to JSON.
 *          Read file, extract ID3 frames, append frames to JSON object.
 *  3. For each subdirectory in current directory:
 *      3a. Perform step 2.
 */


#include <iostream>
#include <filesystem>
#include <fstream>
#include "aiff_reader.h"
#include "id3_parser.h"
#include "util/json.hpp"
#include "library_scanner.h"

int main() {
    const std::string project_root = PROJECT_ROOT;
    const std::string directory_path = project_root + "/music";

    // Recursive directory scanning:
    const nlohmann::json library = libraryToJson(directory_path);
    std::cout << "Recursive library JSON: \n" << library.dump(1) << std::endl;
    // Non-recursive directory scanning:
    const nlohmann::json library2 = libraryToJson(directory_path, false);
    std::cout << "Non recursive library JSON: \n" << library2.dump() << std::endl;

    return 0;
}
