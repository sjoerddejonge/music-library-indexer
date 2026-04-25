//
// Created by Sjoerd de Jonge on 20/04/2026.
//

#ifndef MLI_LIBRARY_SCANNER_H
#define MLI_LIBRARY_SCANNER_H
#include <string>

#include "options.hpp"
#include "nlohmann/json.hpp"

nlohmann::json libraryToJson(const std::filesystem::path& directory_path, const IndexOptions& options);

#endif //MLI_LIBRARY_SCANNER_H
