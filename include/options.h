//
// Created by Sjoerd de Jonge on 22/04/2026.
//

#ifndef MLI_CONFIG_H
#define MLI_CONFIG_H
#include <iostream>
#include <sstream>
#include <chrono>

enum class Output {
    CONSOLE,
    FILE,
    NONE
};

// Struct containing options for the index command.
//
// OPTION           RESULT
// verbose:         More console output during scanning.
// subdirectories:  Include files in subdirectories in scan.
// include_apic:    Include attached picture ID3 frame as base64 string (image stored in text format).
// output:          Where to output the JSON. Either NONE, FILE, or CONSOLE.
// output_path:     String containing file location and name, if output = FILE.
struct IndexOptions {
    bool verbose = false;
    bool subdirectories = true;
    bool include_apic = false;
    Output output_type = Output::FILE;
    std::string output_filename = createFilename();

private:
     static std::string createFilename() {
         const std::string suffix = "mli_snapshot.json";
         std::chrono::sys_time<std::chrono::nanoseconds> now = std::chrono::system_clock::now();
         return std::format("{:%Y%m%d}_{}", now, suffix);
    }
};

#endif //MLI_CONFIG_H
