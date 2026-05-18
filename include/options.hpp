//
// Created by Sjoerd de Jonge on 22/04/2026.
//

#ifndef MLI_CONFIG_H
#define MLI_CONFIG_H
#include <format>
#include <chrono>

#include "program_info.hpp"

/// Output type for the index command.
/// @param CONSOLE:     Print JSON to console.
/// @param FILE:        Write JSON to .json file.
/// @param NONE:        Do not create output.
enum class Output {
    CONSOLE,
    FILE,
    NONE
};

/// Struct containing options for the index command.
/// @param verbose:             More console output during scanning.
/// @param subdirectories:      Include files in subdirectories in scan.
/// @param include_apic:        Include attached picture ID3 frame as base64 string (image stored in text format).
/// @param output_type:         Where to output the JSON. Either NONE, FILE, or CONSOLE.
/// @param output_filename:     String containing filename, if output = FILE.
struct IndexOptions {
    /// More console output during scanning.
    bool verbose = false;
    /// Include files in subdirectories in scan.
    bool subdirectories = true;
    /// Include attached picture ID3 frame as base64 string (image stored in text format).
    bool include_apic = false;
    /// Where to output the JSON. Either NONE, FILE, or CONSOLE.
    Output output_type = Output::FILE;
    /// String containing file location and name, if output = FILE.
    std::string output_filename = createFilename();

private:
    // Create a filename with current UTC date-time stamp and program name.
    static std::string createFilename() {
        const std::string suffix = std::format("{}_snapshot.json", program::name());
        std::chrono::sys_time<std::chrono::nanoseconds> now = std::chrono::system_clock::now();
        return std::format("{:%Y%m%d}_{}", now, suffix);
    }
};

#endif //MLI_CONFIG_H
