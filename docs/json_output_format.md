---
layout: page
title: JSON Output Format
---

[← Return to repo](https://github.com/sjoerddejonge/music-library-indexer)  
[← Return to homepage](docs/index.md)

# JSON Output Format

## Example snapshot
```json
{
  "meta": {
    "directory": "/Users/user/Music/Library",
    "exported_at": "2026-05-09T13:44:49Z",
    "file_count": 2,
    "tool": "mli",
    "version": "0.1.0"
  },
  "songs": [
    {
      "aiff_data": {
        "annotation": "Die Orakel - ORKLVA10",
        "name": "Protectors of the Moss"
      },
      "filename": "14 - Reptant - Protectors of the Moss_toB.aiff",
      "id3_frames": {
        "TALB": "Braindance",
        "TDRC": "2024",
        "TIT2": "Protectors of the Moss",
        "TPE1": "Reptant",
        "TPUB": "Die Orakel"
      },
      "id3_version": "2.4.0",
      "relative_path": "2025-12/14 - Reptant - Protectors of the Moss.aiff"
    },
    {
      "aiff_data": {
        "comments": [
          {
            "marker_id": 0,
            "text": "Processed by SoX"
          }
        ]
      },
      "filename": "Amir Alexander _ - Innovation.aiff",
      "id3_frames": {
        "APIC": [
          {
            "description": "cover",
            "mime_type": "image/jpeg",
            "picture_data": "/9j/4AAQSkZJRgABAQSkZAQSkZAQSkZ...",
            "picture_type": 3
          }
        ],
        "COMM": [
          {
            "comment": "Visit https://deepvibesrecordings.bandcamp.com",
            "description": "",
            "language": "eng"
          }
        ],
        "TALB": "Outsider Music EP",
        "TIT2": "Innovation",
        "TPE1": "Amir Alexander",
        "TPE2": "Amir Alexander",
        "TRCK": "1",
        "TYER": "2012"
      },
      "id3_version": "2.4.0",
      "relative_path": "2025-12/Amir Alexander _ - Innovation.aiff"
    }
  ]
}
```
## Meta block


[← Return to repo](https://github.com/sjoerddejonge/music-library-indexer)  
[← Return to homepage](docs/index.md)
