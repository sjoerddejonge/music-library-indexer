---
layout: page
title: Supported ID3v2 frames
---
[Home](index.md) `>` [Supported ID3v2 frames](supported_id3_frames.md)

# Supported ID3v2 frames
ID3v2 frames are represented as structs. A struct can represent a category of frames (such as text
information frames), or a single frame (such as the APIC frame).  

## Base ID3 frame struct
Each struct inherits from the base struct `Id3Frame` and should implement (override) the `toJson` 
method.
```c++
struct ID3Frame {
    ID3FrameHeader header{};
    virtual ~ID3Frame() = default;
    [[nodiscard]] virtual nlohmann::json toJson() const = 0;
    virtual bool isArrayType() {
        return false;
    };
};
```
A type of frame has the following properties:  
* `ID3FrameHeader` - The header for the frame, containing the `frame_id`, `size`, and `flags`
* `nlohmann::json toJson()` - Method that returns a JSON with the frame contents
* `bool isArrayType()` - Method that returns whether the frame is an array type of frame, default
= `false`

Frames that are not of the array type should follow this structure: 
```json
"frame_id": "contents of the frame"
```  
Frames that are of the array type are structured like:
```json
"frame_id": [
  {contents of frame 1},
  {contents of frame 2}
]
```

## Text information frames
`isArrayType()` = `false`  

Taken from the official [documentation](https://id3.org/id3v2.4.0-frames#:~:text=Text%20information%20frames,-The) at id3.org:
<details>

<summary>All supported text information frames</summary>

### Identification frames
```text
TIT1
    The 'Content group description' frame is used if the sound belongs to
    a larger category of sounds/music. For example, classical music is
    often sorted in different musical sections (e.g. "Piano Concerto",
    "Weather - Hurricane").

TIT2
    The 'Title/Songname/Content description' frame is the actual name of
    the piece (e.g. "Adagio", "Hurricane Donna").

TIT3
    The 'Subtitle/Description refinement' frame is used for information
    directly related to the contents title (e.g. "Op. 16" or "Performed
    live at Wembley").

TALB
    The 'Album/Movie/Show title' frame is intended for the title of the
    recording (or source of sound) from which the audio in the file is
    taken.

TOAL
    The 'Original album/movie/show title' frame is intended for the title
    of the original recording (or source of sound), if for example the
    music in the file should be a cover of a previously released song.

TRCK
    The 'Track number/Position in set' frame is a numeric string
    containing the order number of the audio-file on its original
    recording. This MAY be extended with a "/" character and a numeric
    string containing the total number of tracks/elements on the original
    recording. E.g. "4/9".

TPOS
    The 'Part of a set' frame is a numeric string that describes which
    part of a set the audio came from. This frame is used if the source
    described in the "TALB" frame is divided into several mediums, e.g. a
    double CD. The value MAY be extended with a "/" character and a
    numeric string containing the total number of parts in the set. E.g.
    "1/2".

TSST
    The 'Set subtitle' frame is intended for the subtitle of the part of
    a set this track belongs to.

TSRC
    The 'ISRC' frame should contain the International Standard Recording
    Code [ISRC] (12 characters).
```

### Involved persons frames
```text
TPE1
    The 'Lead artist/Lead performer/Soloist/Performing group' is
    used for the main artist.

TPE2
    The 'Band/Orchestra/Accompaniment' frame is used for additional
    information about the performers in the recording.

TPE3
    The 'Conductor' frame is used for the name of the conductor.

TPE4
    The 'Interpreted, remixed, or otherwise modified by' frame contains
    more information about the people behind a remix and similar
    interpretations of another existing piece.

TOPE
    The 'Original artist/performer' frame is intended for the performer
    of the original recording, if for example the music in the file
    should be a cover of a previously released song.

TEXT
    The 'Lyricist/Text writer' frame is intended for the writer of the
    text or lyrics in the recording.

TOLY
    The 'Original lyricist/text writer' frame is intended for the
    text writer of the original recording, if for example the music in
    the file should be a cover of a previously released song.

TCOM
    The 'Composer' frame is intended for the name of the composer.

TMCL
    The 'Musician credits list' is intended as a mapping between
    instruments and the musician that played it. Every odd field is an
    instrument and every even is an artist or a comma delimited list of
    artists.

TIPL
    The 'Involved people list' is very similar to the musician credits
    list, but maps between functions, like producer, and names.

TENC
    The 'Encoded by' frame contains the name of the person or
    organisation that encoded the audio file. This field may contain a
    copyright message, if the audio file also is copyrighted by the
    encoder.
```

### Derived and subjective properties frames

</details>  

[← Return to repo](https://github.com/sjoerddejonge/music-library-indexer)   
