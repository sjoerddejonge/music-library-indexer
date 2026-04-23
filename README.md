# [WIP] Music library indexer
A CLI app that reads all AIFF files in the current (and sub-)
directories, and extracts their ID3 tags. 
Still a **work-in-progress**.  

## 1. Getting started
In `/music` you will find the some example `.aiff` files that
all have tags.

Running this program will scan through the directories in
`/music` and extract the ID3 tags.

## 2. Features
* Supported music files
  * `.aiff`
* ID3 frames
  * Text frames
    * `[TIT1]` Content group description
    * `[TIT2]` Song title
    * `[TIT3]` Subtitle/Description refinement
    * `[TALB]` Album name
    * `[TPE1]` Artist
    * `[TPE2]` Band/Orchestra/Accompaniment
    * `[TPE3]` Conductor
    * `[TPE4]` Interpreted, remixed, or otherwise modified by
    * `[TDRC]` Recording time
    * `[TPUB]` Record label
    * `[TXXX]` User defined text frame
    * `[T...]` All other text frames are also (likely) supported
  * `[COMM]` Comments
  * `[APIC]` Album cover image encoded in JSON (Base64)
* Supported ID3 frame text encoding
  * `ISO-8859-1`
  * `UTF-8`
  * `UTF-16`
  * `UTF-16BE`
* Output
  * `JSON`

## 3. Approach
1. Scan all files in current directory
2. Loop through all `.aiff` files
3. Loop through all subdirectories and start at step 1

### For each `.aiff` file  
1. Read AIFF file  
2. Scan all chunks  
3. Locate ID3 chunk
4. Parse ID3 header 
5. Scan all frames 
6. Append frames to JSON  

All frames will be added to JSON. Later on I can decide which 
I find relevant (for example artist, title, genre, label),

## 4. Contents
`main.cpp`  

`src/library_scanner.cpp`  
Scan through a directory looking for supported filetypes.
  
`src/aiff_reader.cpp`  
Read `.aiff` file, scan chunks to find ID3 chunk,
pass it to the id3_parser.

`src/id3_parser.cpp`  
Parse ID3 header, scan and read frames.
  
`include/util/helpers.h`  
Header-only helper functions.  
  
`include/util/base64.h`  
Header-only Base64 encoder and decoder. Stand-alone,
free to use in your projects.  

`include/util/json.hpp`  
Header-only json library by github.com/nlohmann 
under MIT License.

## 5. Milestones  
### For the MVP
`[x]` Read FORM chunk + all chunk headers (ID + size).  
`[x]` Read ID3 tag header.  
`[x]` Scan and read ID3 frames.  
`[x]` Write a Base64 encoder.  
`[x]` (Optional) write a Base64 decoder.  
`[x]` Parse ID3 frames.  
`[x]` Append ID3 frames to JSON.      
`[x]` Iterate through all files in directory to extract tags.  
`[x]` Iterate through all subfolders to extract all tags.   
`[ ]` Accept input arguments (`mli index`, `mli index --verbose`).  
`[x]` Write the JSON to a file.  

### Wishlist
`[x]` Decouple the ID3 parsing logic from the AIFF reader.  
`[ ]` Fix code TODOs  
`[ ]` Add support for ALL ID3 frames.  
`[ ]` Add support for other music files that can contain ID3.  
`[x]` Add support for `UTF-16(BE)` text encoding.
