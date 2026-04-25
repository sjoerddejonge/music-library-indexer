//
// Created by Sjoerd de Jonge on 24/04/2026.
//

#include "program_info.hpp"

#include <filesystem>

static std::string program_name;    // The runtime name of this program, retrieve through 'program::name()'
static std::string program_version; // <major>.<minor>.<patch>

// Initialize the runtime program name.
void program::init(const int argc, char *argv[]) {
    if (argc > 0 && argv[0]) {
        program_name = std::filesystem::path(argv[0]).filename().string();
    }
    program_version = PROJECT_VERSION;
}

// Get the runtime name of the program (name of the binary file).
const std::string& program::name() {
    return program_name;
}

const std::string& program::version() {
    return program_version;
}
