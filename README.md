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

<!-- TOC -->
* [Music library indexer](#music-library-indexer)
  * [1. Design goals](#1-design-goals)
  * [2. Installation](#2-installation)
    * [(a) Add to PATH](#a-add-to-path)
    * [(b) Forking/cloning](#b-forkingcloning)
    * [(c) Building the app](#c-building-the-app)
  * [3. How to use](#3-how-to-use)
    * [Running from PATH](#running-from-path)
    * [Running from your IDE](#running-from-your-ide)
    * [Output](#output)
  * [4. Features](#4-features)
    * [Supported music file formats](#supported-music-file-formats)
    * [Supported output file formats](#supported-output-file-formats)
    * [Supported ID3 frames](#supported-id3-frames)
    * [Supported ID3 frame text encoding](#supported-id3-frame-text-encoding)
  * [5. Project structure](#5-project-structure)
    * [Adding new file format readers](#adding-new-file-format-readers)
    * [Adding new commands](#adding-new-commands)
    * [Console output](#console-output)
  * [6. Milestones](#6-milestones)
    * [For version 1.0.0 (MVP)](#for-version-100-mvp)
    * [Wishlist](#wishlist)
<!-- TOC -->

## 1. Design goals
The first and foremost goal of this project is to improve my software
engineering skills by learning modern C++ and best practices for
software development. Therefore, this repository is also meant as a 
personal portfolio piece!  
The second goal is to build a tool that is personally useful for managing
a music library. Other goals are:
* **Intuitive CLI interface**. The Unix-style CLI interface should be
  clear to use for users of tools like `git`.
* **Format-agnostic design**. Adding support for new file formats is as
  simple as adding a `flac_reader.hpp` and `flac_reader.cpp`.
* **Safety**. Because this tool only **reads** your files, it poses
  no risk for your library. Writing ID3 tags is (currently) not included
  in the scope as writing ID3 tags is not trivial and poses a risk for
  corrupting your entire music library.

## 2. Installation
Choose any of the following options, depending on intended usage:

* **(a) Add to PATH**. This allows you to run this app from anywhere
  inside your terminal through `mli <command> [<args>]`.
* **(b) Fork/clone the repo**. If you wish to alter the source code,
  or build it yourself, you can fork or clone this repo.
* **(c) Building the app**. Instead of downloading the binary from here,
  you can build it yourself.

### (a) Add to PATH
Download the binary from the lastest release on this page (`CMD/CTRL +
 F` "Releases") and add to any folder within your PATH.

**On macOS**  
Download the binary, and then copy it to `/usr/local/bin/`:
```commandline
sudo cp ~/Downloads/mli /usr/local/bin/mli
```
Or copy-paste it with Finder.

### (b) Forking/cloning
Navigate to your project folder in the terminal and run:
```commandline
git clone https://github.com/sjoerddejonge/music-library-indexer.git
```

### (c) Building the app
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

### Output
The output will be a `JSON`. Here is an example of the output:
```json
{
  "meta": {
    "directory": "/Users/username/Music",
    "exported_at": "2026-04-25T15:15:02Z",
    "file_count": 2,
    "tool": "mli",
    "version": "1.0.0"
  },
  "songs": [
    {
      "COMM": {
        "": "This is a comment!"
      },
      "TALB": "Moon Safari",
      "TCON": "Electronic",
      "TDRC": "1998",
      "TIT2": "La Femme D'argent",
      "TPE1": "Air",
      "TRCK": "1"
    },
    {
      "COMM": {
        "": "A remix of the previous track"
      },
      "TALB": "Blue Moon Safari",
      "TCON": "Electronic",
      "TDRC": "2025",
      "TIT2": "La Femme D'argent (Vegyn Remix)",
      "TPE1": "Air",
      "TPE4": "Vegyn",
      "TRCK": "6"
    }
  ]
}
```
The time under `"exported_at"` is always in UTC. The value for `"tool"`
is always `"mli"`, even if the user has renamed their binary.

## 4. Features
This app supports reading both `id3v2.3.0` and `id3v2.4.0` tags. It 
also has an option to include the album cover in the `JSON`. Because a
`JSON` can only contain text, I wrote a 
[Base64 encoder](https://en.wikipedia.org/wiki/Base64). This encodes 
the binary pixel data to a string of 64 possible text characters. 

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
* `main.cpp`  
* `src/`
  * `commands.cpp` All CLI commands (index, help).
  * `program_info.cpp` Stores the runtime program name.   
  * `library_scanner.cpp` Scan through directory, used by `mli index`.
  * `aiff_reader.cpp` Read .aiff files.
  * `id3_parser.cpp` Parse ID3 tag. 
* `include/`
  * `util/`
    * `helpers.hpp` Header-only helper functions.
    * `base64.hpp` Header-only Base64 encoder and decoder.
  * `nlohmann/`
    * `json.hpp` Header-only json library by github.com/nlohmann
  * `options.hpp` Header-only command options.

### Adding new file format readers
Adding support for new music file formats is as easy as creating a 
separate implementation file, for example: `flac_reader.cpp`. It 
should have a function that accepts an `std::ifstream&` and reads 
up until the start of ID3 header:
```c++
void locateId3(std::ifstream& fin);
```

This function can then be called in the function `libraryToJson()`
in `library_scanner.cpp`:
```c++
// Lambda function for scanning the directories (recursive or not):
auto scan = [&](const auto& iterator) {
    for (auto const& dir_entry : iterator) {
        if (dir_entry.is_directory() && options.subdirectories) {
            std::cout << "Reading files in: " << dir_entry.path() << "\n";
        }

        /*
         * Add newly supported file formats here:
         */

        // AIFF files
        if (dir_entry.path().extension() == ".aiff" || dir_entry.path().extension() == ".aif") {
            std::ifstream fin{ dir_entry.path(), std::ios_base::binary }; // Create an if-stream to open the file.
            if (!fin) {
                std::cerr << "Failed to open: " << dir_entry.path() << "\n";
                continue;
            }
            try {
                locateId3(fin); // Skip ifstream to the start of the ID3 tag.
                const nlohmann::json song = id3ToJson(fin, options);
                if (!song.is_null()) library.push_back(song);
            }
            catch (const std::exception& e) {
                std::cerr << "Error occurred: " << e.what() << "\n";
            }
            fin.close();
        }
    }
};
```

### Adding new commands
Each command (such as `mli index` or `mli help`) is added as a function 
in `src/commands.cpp` under the `commands` namespace. New commands should 
follow this structure.

### Console output
When printing text to the user in the terminal, all references to the 
name of the app use `program::name()` (`#include program_info.hpp`) 
instead of hard-coding the name:
```c++
std::cout << std::format("This is program is called {}.\n", program::name());
```
```terminaloutput
This program is called mli.
```
This allows the user to rename the binary in case of conflict with other
binaries using `mli <command>` to call their program, while keeping all
functionality like help text.

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
* `[ ]` Improve argument parsing to be more Unix-like
* `[ ]` Fix code TODOs
* `[ ]` Add support for **ALL** ID3 frames
* `[ ]` Support more music file formats:
  * `[ ]` Add support for `.flac` music files
  * `[ ]` Add support for `.mp3` music files
* `[x]` Decouple the ID3 parsing logic from the AIFF reader
