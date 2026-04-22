//
// Created by Sjoerd de Jonge on 20/04/2026.
//

#ifndef MLI_LIBRARY_SCANNER_H
#define MLI_LIBRARY_SCANNER_H
#include <string>

#include "options.h"
#include "util/json.hpp"

nlohmann::json libraryToJson(const std::string& directory_path, const IndexOptions& options);

#endif //MLI_LIBRARY_SCANNER_H
