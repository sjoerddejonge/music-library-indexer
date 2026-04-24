//
// Created by Sjoerd de Jonge on 24/04/2026.
//

#ifndef MLI_COMMANDS_H
#define MLI_COMMANDS_H
#include <filesystem>

#include "options.h"

/*
 *      All commands supported by the program.
 */

namespace commands {
    void index(const std::filesystem::path& directory_path, const IndexOptions& options);
    void help();
}

#endif //MLI_COMMANDS_H
