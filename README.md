# Music library indexer
A CLI app that reads all AIFF files in the current (
and sub-) directories. Still a work-in-progress.  

## 1. Getting started
In `/music` you will find a sample .aiff file. 
Running this program will read its ID3 tags.

## 2. Approach
1. Scan all files in current directory
2. Loop through all .aiff files
3. Loop through all subdirectories and start at step 1

### For each .aiff file  
1. Read AIFF file  
2. Scan all chunks  
3. Locate ID3 chunk
4. Parse ID3 header 
5. Scan all frames 
6. Append frames to JSON  

All frames will be added to JSON. Later on I can decide which I find relevant 
(for example artist, title, genre, label),

## 3. Structure
`main.cpp`  
  
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

## 4. Milestones  
[x] Read FORM chunk  
[x] Read all chunk headers (ID + size)  
[x] Read ID3 tag header  
[x] Scan ID3 frames  
[x] Read ID3 frames  
[x] Write a Base64 encoder  
[x] (Optional) Write a Base64 decoder  
[ ] Parse ID3 frames  
[ ] Append ID3 frames to JSON     
[ ] Accept input arguments (`mli index`, `mli index -verbose`)  
[ ] Iterate through all files in directory to extract tags  
[ ] Iterate through all subfolders to extract all tags  
