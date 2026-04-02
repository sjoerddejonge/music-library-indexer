/**
 *  Music Library Indexer
 *  @author Sjoerd de Jonge
 *  2026-03-28
 *
 *  Reads .aiff music files in target folder
 *  Extracts metadata
 *  Structures metadata for the entire target folder
 */


#include <iostream>
#include "aiff_reader.h"

int main() {
    std::string filename = "/Users/sjoerd/git/music-library-indexer/music/Unherluferlick.aiff";
    read(filename);
    return 0;
}