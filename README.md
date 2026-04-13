## Approach
1. Read AIFF file  
2. Scan all chunks  
3. Locate ID3 chunk  
4. Parse ID3 chunk
5. Read ID3 header
6. Scan all frames
7. Read target frames  

Target frames contain the information that I want to extract.  
These are:
- artist
- song title
- key
- bpm
- record label

## Milestones  
[x] Read FORM chunk  
[x] Read all chunk headers (ID + size)  
[x] Read ID3 tag header  
[ ]   
...  
[ ] Read ID3v2.4.0 tags  
...  
[ ] Save ID3v2.4.0 tags to (JSON / CSV / ???)  
...  
[ ] Iterate through all files in directory to extract tags  
[ ] Iterate through all subfolders to extract all tags  
