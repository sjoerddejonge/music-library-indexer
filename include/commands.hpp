//
// Created by Sjoerd de Jonge on 24/04/2026.
//

#ifndef MLI_COMMANDS_H
#define MLI_COMMANDS_H
#include <filesystem>

#include "options.hpp"

/*
 *      All commands supported by the program.
 */

namespace commands {
    /**
     * @brief Scan the directory for compatible files, extract ID3 tags for each file, then append to JSON.
     * @param directory_path Path to the directory to scan
     * @param options IndexOptions to configure options for this command, see include/options.hpp
     */
    void index(const std::filesystem::path& directory_path, const IndexOptions& options);

    /**
     * @brief Print help text for instructions on how to use this program.
     */
    void help();
}

#endif //MLI_COMMANDS_H
