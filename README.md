# Music library indexer
A CLI app that reads all AIFF files in the current (and sub-) directories.  

## Approach
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

## Milestones  
[x] Read FORM chunk  
[x] Read all chunk headers (ID + size)  
[x] Read ID3 tag header  
[x] Scan ID3 frames  
[ ] Read target ID3 frames  
[ ] Append ID3 frames to JSON  
[ ] Accept input arguments (`mli index`, `mli index -verbose`)  
[ ] Iterate through all files in directory to extract tags  
[ ] Iterate through all subfolders to extract all tags  
