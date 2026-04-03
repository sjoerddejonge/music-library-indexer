cmake_minimum_required(VERSION 4.2)
project(mli) # Accessible through ${PROJECT_NAME}

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} main.cpp
        include/aiff_reader.h
        include/util/helpers.h
        src/aiff_reader.cpp
        include/id3_parser.h
        src/id3_parser.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)