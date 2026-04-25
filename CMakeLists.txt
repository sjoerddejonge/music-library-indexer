cmake_minimum_required(VERSION 4.2)
project(mli
        VERSION 1.0.0
)
# Project name is accessible through ${PROJECT_NAME}
# Project version through ${PROJECT_VERSION} throughout this file

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} main.cpp
        include/aiff_reader.hpp
        include/util/helpers.hpp
        src/aiff_reader.cpp
        include/id3_parser.hpp
        src/id3_parser.cpp
        include/util/base64.hpp
        include/nlohmann/json.hpp
        include/library_scanner.hpp
        src/library_scanner.cpp
        include/options.hpp
        src/commands.cpp
        include/commands.hpp
        include/program_info.hpp
        src/program_info.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
    PROJECT_VERSION="${PROJECT_VERSION}"
    $<$<CONFIG:Debug>:DEBUG>
)
target_include_directories(${PROJECT_NAME} PRIVATE include)