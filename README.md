# Music library indexer
A CLI app that exports metadata from your music library to JSON

Running this app with the `mli index` command will scan all files
in the current working directory and its subdirectories. For each
supported music file that is found, it will read the binary data
to find the ID3 tag, and extract all information from the tag.
The information is added to a `JSON` that is written to the current
working directory, called a snapshot.

Uses the [nlohmann/json](https://github.com/nlohmann/json) library.
All other files are written by me.

1. [Design goals](#1-design-goals)
2. [Installation](#2-installation)
3. [How to use](#3-how-to-use)
4. [Features](#4-features)
5. [Project structure](#5-project-structure)
6. [Milestones](#6-milestones)

## 1. Design goals
The first and foremost goal of this project is to improve my software
engineering skills by learning modern C++ and best practices for
software development. The second goal is to build a tool that is
personally useful for managing a music library.
Other goals are:
* **Intuitive CLI interface**. The Unix-style CLI interface should be
  clear to use for users of tools like `git`.
* **Format-agnostic design**. Adding support for new file formats is as
  simple as adding a `flac_reader.h` and `flac_reader.cpp`.
* **Safety**. Because this tool only **reads** your files, it poses
  no risk for your library. Writing ID3 tags is (currently) not included
  in the scope as writing ID3 tags is not trivial and poses a risk for
  corrupting your entire music library.

## 2. Installation
Choose any of the following options, depending on intended usage:
* **Add to PATH**. This allows you to run this app from anywhere
  inside your terminal through `mli <command> [<args>]`.
* **Fork/clone the repo**. If you wish to alter the source code,
  or build it yourself, you can fork or clone this repo.
* **Building the app**. Instead of downloading the binary from here,
  you can build it yourself.

### Add to PATH
Download the binary from the lastest release on this page (`CMD/CTRL +
 F` "Releases") and add to any folder within your PATH.

**On macOS**  
Download the binary, and then copy it to `/usr/local/bin/`:
```commandline
sudo cp ~/Downloads/mli /usr/local/bin/mli
```
Or copy-paste it with Finder.

### Forking/cloning
Navigate to your project folder in the terminal and run:
```commandline
git clone https://github.com/sjoerddejonge/music-library-indexer.git
```

### Building the app
Build as 'Release' (`-DCMAKE_BUILD_TYPE=Release`) to ensure that
it uses the correct default path when running the `index` command.
The 'Debug' build type uses the path to the project folder as
default directory, while 'Release' uses the current working
directory.

## 3. How to use

### Running from PATH
Run any of the following commands in the terminal.
```
mli index
# Runs the indexer on the current directory and subdirectories

mli index <PATH>
# Runs the indexer on specified directory

mli index -a
# Include attached picture (APIC) frames in JSON encoded as base64

mli index -v
# Include verbose console output

mli index --shallow
# Scan target directory only (non-recursive; excluding subdirectories)
```
### Running from your IDE

Inside the `/music` directory you will find the some example
`.aiff` files that have ID3 tags. Running the app inside your
IDE as 'Debug' (`-DCMAKE_BUILD_TYPE=Debug`) will target that music
folder. Keep in mind that you have to provide the input argument
`index`, otherwise it will only print the help text and exit.

To do so in CLion, go to `Run > Edit 
Configurations...` and add `index` to the 'Program Arguments' field.

## 4. Features
### Supported music file formats
* `.aiff`

### Supported output file formats
* `.json`

### Supported ID3 frames
```text
Text frames
[TIT1] - [TIT3]     Titles
[TALB]              Album name
[TPE1] - [TPE3]     Artist and contributors
[TCON]              Genre
[TDRC]              Recording time
[TPUB]              Record label
[TXXX]              User defined text
[T...]              All other text frames are also supported

Other frames
[COMM]              Comments
[APIC]              Attached picture, will be encoded as text
```
### Supported ID3 frame text encoding
* `ISO-8859-1`
* `UTF-8`
* `UTF-16`
* `UTF-16BE`

## 5. Project structure
```text
main.cpp

src/commands.cpp
    All available CLI commands (index, help)

src/program_info.cpp
    Stores the runtime program name

src/library_scanner.cpp
    Scan through a directory looking for supported filetypes

src/aiff_reader.cpp
    Read .aiff file, scan chunks to find ID3 chunk, pass it to the 
    id3_parser

src/id3_parser.cpp
    Parsing of the ID3 tag

include/util/helpers.hpp
    Header-only helper functions.

include/util/base64.hpp
    Header-only Base64 encoder and decoder. Stand-alone, free to use 
    in your projects.

include/options.hpp
    Header-only file that contains definitions of command options.

include/nlohmann/json.hpp
    Header-only json library by github.com/nlohmann under MIT License.
```

## 6. Milestones
### For version 1.0.0 (MVP)
* `[x]` Read `.aiff` file to locate ID3 tag
* `[x]` Read ID3 header
* `[x]` Scan and read ID3 frames
* `[x]` Write a Base64 encoder
* `[x]` (Optional) write a Base64 decoder
* `[x]` Parse ID3 frames
* `[x]` Add support for `UTF-16(BE)` text encoding
* `[x]` Append ID3 frames to JSON
* `[x]` Iterate through all files in a directory to extract tags
* `[x]` Accept input arguments (`mli index`, `mli index --verbose`)
* `[x]` Write the JSON to a file
* `[ ]` Fix bugs and testing

### Wishlist
* `[ ]` Improve argument parsing
* `[ ]` Fix code TODOs
* `[ ]` Add support for ALL ID3 frames
* `[ ]` Support more music file formats:
  * `[ ]` Add support for `.flac` music files
  * `[ ]` Add support for `.mp3` music files
* `[x]` Decouple the ID3 parsing logic from the AIFF reader
