/**
 *  Music Library Indexer
 *  @author Sjoerd de Jonge
 *  2026-03-28
 *
 *  Method:
 *  1. Create empty JSON string.
 *  2. Loop through all files in current directory.
 *      2a. For each .aiff file:
 *          Add new object to JSON.
 *          Read file, extract ID3 frames, append frames to JSON object.
 *  3. For each subdirectory in current directory:
 *      3a. Perform step 2.
 */


#include <iostream>
#include <filesystem>
#include "aiff_reader.h"
#include "util/base64.h"
#include "util/json.hpp"

int main() {
    const std::string project_root = PROJECT_ROOT;
    const std::string filename = project_root + "/music/sample_break.aiff";

    // JSON to store the song tag data
    nlohmann::json song;

    std::map<std::string, std::vector<std::string> > id3_tag = extractID3(filename, song);
    std::cout << song.dump() << std::endl;
    // for (const auto& [key, value] : id3_tag) {
    //     for (const auto & i : value) {
    //         if (key == "APIC") continue;
    //         std::cout << '[' << key << "] = " << i << "; " << std::endl;
    //     }
    // }
    return 0;
}
