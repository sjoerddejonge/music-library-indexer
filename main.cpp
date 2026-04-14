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
#include "aiff_reader.h"
#include "util/base64.h"

int main() {
    std::string filename = "/Users/sjoerd/git/music-library-indexer/music/Unherluferlick.aiff";
    std::map<std::string, std::vector<std::string>> id3_tag;
    id3_tag = extractID3(filename);

    for (const auto& [key, value] : id3_tag) {
        for (const auto & i : value) {
            std::cout << '[' << key << "] = " << i << "; " << std::endl;
        }
    }
    return 0;
}
