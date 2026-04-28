cmake_minimum_required(VERSION 4.2)

# IMPORTANT: Changing PROJECT_NAME (mli) requires a major PROJECT_VERSION update!!!
project(mli
        VERSION 0.1.0
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

# Only for Windows release builds:
if(WIN32 AND CONFIG:Release)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_NAME="${PROJECT_NAME}"
    $<$<CONFIG:Debug>:DEBUG>
)
target_include_directories(${PROJECT_NAME} PRIVATE include)