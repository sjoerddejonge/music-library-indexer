## Approach
* Read AIFF file, scan all chunks
* Parse ID3 chunk
  * Read header
  * Scan all frames
    * Read target frames

Target frames contain the information that I want to extract. 
For example: artist, song title, key, bpm, record label, etc.

## Milestones  
[x] Read FORM chunk  
[x] Read all chunk headers (ID + size)  
[x] Read ID3 tag header  
[ ]   
...  
[ ] Read ID3v2.4.0 tags  
...  
[ ] Write ID3v2.4.0 tags to (JSON / CSV / ???)  
...  
[ ] Iterate through all files in directory to extract tags  
[ ] Iterate through all subfolders to extract all tags  
