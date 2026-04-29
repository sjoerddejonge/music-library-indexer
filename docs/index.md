---
layout: default
title: Music Library Indexer docs
---

# Documentation

Getting started with writing documentation!

# API/Output
## JSON
The currently only supported form of output is a JSON file. The 
file consists of a `"meta"` section and a `"songs"` section. All
JSON keys should be in `snake_case`.

### Example
Below you will find an example of a snapshot of a music library
directory consisting of just two songs.

<details>

<summary>Example JSON</summary>

```json
{
  "meta": {
    "directory": "/Users/username/Music",
    "exported_at": "2026-04-25T15:15:02Z",
    "file_count": 2,
    "tool": "mli",
    "version": "0.1.0"
  },
  "songs": [
    {
      "filename": "la-femme-dargent.aiff",
      "id3_frames": {
        "COMM": {
          "comment": "This is a comment",
          "description": "",
          "language": "eng"
        },
        "TALB": "Moon Safari",
        "TCON": "Electronic",
        "TDRC": "1998",
        "TIT2": "La Femme D'argent",
        "TPE1": "Air",
        "TRCK": "1"
      },
      "id3_version": "2.3.0",
      "relative_path": "Air/la-femme-dargent.aiff"
    },
    {
      "filename": "la-femme-dargent (remix).aif",
      "id3_frames": {
        "COMM": {
          "comment": "This is track is a remix",
          "description": "",
          "language": "eng"
        },
        "TALB": "Blue Moon Safari",
        "TCON": "Electronic",
        "TDRC": "2025",
        "TIT2": "La Femme D'argent (Vegyn Remix)",
        "TPE1": "Air",
        "TPE4": "Vegyn",
        "TRCK": "6"
      },
      "id3_version": "2.4.0",
      "relative_path": "Vegyn/la-femme-dargent (remix).aif"
    }
  ]
}
```
</details>

### Meta
The `"meta"` section contains the metadata of the snapshot.

* `"directory"` - The (upper) target directory that the snapshot 
was taken of.
* `"exported_at` - The timestamps at which the snapshot was taken, in
`YYYY-MM-DDTHH:MM:SSZ` format, with `T` indicating the time section
and `Z` signalling that the timezone is UTC.
* `"file_count"` - The amount of music files included in this snapshot, 
as a JSON numeric.
* `"tool""` - The internal CMake `PROJECT_NAME`, when the app was built. 
This should always be `"mli"`, even if the user renames their binary and 
runs the app under a different name.
* `"version"` - The version of the Music Library Indexer that was used 
to make this snapshot.

### Songs
The `"songs"` section has `"meta" > "file_count"` amount of songs, with
each song containing...

* `"filename"` - The filename, with extension, without folders.
* `"id3_frames"` - The contents of each ID3 frame that the app supports. 
See [ID3 Frames](#id3-frames).
* `"id3_version"` - The version of the ID3 tag.
* `"relative_path"` - The path of the file, excluding the base path 
defined under `"meta" > "directory"`.

### ID3 Frames
This section lists the supported ID3 frames. All frames start with 
their ID3 frame ID, consisting of four letters.

#### Text information frames
All frames starting with a `T`, excluding `TXXX`, are text information 
frames. These hold exactly one string, with `"TALB"` as example:
```json
"TALB": "Album name"
```
#### TXXX frames
A `TXXX` frame is a user defined text frames and follow the 
`"description": "value"` structure. A file can contain more 
than one `TXXX` frame, but only for each unique `"description"`. In 
the following example, the `"description"` is `"ISRC"`, and the value 
is `"CA-M26-01-50003"`:
```json
"TXXX": {
  "ISRC": "CA-M26-01-50003"
}
```
When there is more than one `"TXXX"` frame, the different frames are 
added under the same key (`"TXXX"`), using their unique description as
sub-key.

#### COMM frames
A comment frame, `COMM`, is used for adding comments to songs. Besides 
the comment, it may contain a description and a language (using 3 
characters for the language). 
```json
"COMM": {
  "comment": "This is a comment",
  "description": "",
  "language": "eng"
```

#### APIC frames
An `APIC` (attached picture) frame is optional when the indexer is 
executed with the `mli index -a` argument. The picture data that is
embedded in the ID3 tag is encoded to text using Base64, resulting in 
a long string of text under key `"picture_data"`.
```json
"APIC": {
  "description": "",
  "mime_type": "image/png",
  "picture_data": "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",
  "picture_type": 0
},
```