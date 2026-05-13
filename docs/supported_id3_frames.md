---
layout: page
title: Music Library Indexer
---

<div style="display: flex; justify-content: space-between;">
    <span>
        <a href="{{ site.baseurl }}">{{ site.index_page_name }}</a> <code>></code> 
        <a href="/supported_id3_frames.html">Supported ID3v2 frames</a>
    </span>
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>

---

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

<summary>Identification frames</summary>

<pre><code>TIT1
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
</code></pre>
</details>

<details>

<summary>Involved persons frames</summary>

<pre><code>TPE1
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
</code></pre>

</details>

<details>

<summary>Derived and subjective properties frames</summary>

<pre><code>  TBPM
   The 'BPM' frame contains the number of beats per minute in the
   main part of the audio. The BPM is an integer and represented as a
   numerical string.

  TLEN
   The 'Length' frame contains the length of the audio file in
   milliseconds, represented as a numeric string.

  TKEY
   The 'Initial key' frame contains the musical key in which the sound
   starts. It is represented as a string with a maximum length of three
   characters. The ground keys are represented with "A","B","C","D","E",
   "F" and "G" and halfkeys represented with "b" and "#". Minor is
   represented as "m", e.g. "Dbm" $00. Off key is represented with an
   "o" only.

  TLAN
   The 'Language' frame should contain the languages of the text or
   lyrics spoken or sung in the audio. The language is represented with
   three characters according to ISO-639-2 [ISO-639-2]. If more than one
   language is used in the text their language codes should follow
   according to the amount of their usage, e.g. "eng" $00 "sve" $00.

  TCON
   The 'Content type', which ID3v1 was stored as a one byte numeric
   value only, is now a string. You may use one or several of the ID3v1
   types as numerical strings, or, since the category list would be
   impossible to maintain with accurate and up to date categories,
   define your own. Example: "21" $00 "Eurodisco" $00

   You may also use any of the following keywords:
   
     RX  Remix
     CR  Cover

  TFLT
   The 'File type' frame indicates which type of audio this tag defines.
   The following types and refinements are defined:

     MIME   MIME type follows
     MPG    MPEG Audio
       /1     MPEG 1/2 layer I
       /2     MPEG 1/2 layer II
       /3     MPEG 1/2 layer III
       /2.5   MPEG 2.5
       /AAC   Advanced audio compression
     VQF    Transform-domain Weighted Interleave Vector Quantisation
     PCM    Pulse Code Modulated audio

   but other types may be used, but not for these types though. This is
   used in a similar way to the predefined types in the "TMED" frame,
   but without parentheses. If this frame is not present audio type is
   assumed to be "MPG".

  TMED
   The 'Media type' frame describes from which media the sound
   originated. This may be a text string or a reference to the
   predefined media types found in the list below. Example:
   "VID/PAL/VHS" $00.

    DIG    Other digital media
      /A    Analogue transfer from media

    ANA    Other analogue media
      /WAC  Wax cylinder
      /8CA  8-track tape cassette

    CD     CD
      /A    Analogue transfer from media
      /DD   DDD
      /AD   ADD
      /AA   AAD

    LD     Laserdisc

    TT     Turntable records
      /33    33.33 rpm
      /45    45 rpm
      /71    71.29 rpm
      /76    76.59 rpm
      /78    78.26 rpm
      /80    80 rpm

    MD     MiniDisc
      /A    Analogue transfer from media

    DAT    DAT
      /A    Analogue transfer from media
      /1    standard, 48 kHz/16 bits, linear
      /2    mode 2, 32 kHz/16 bits, linear
      /3    mode 3, 32 kHz/12 bits, non-linear, low speed
      /4    mode 4, 32 kHz/12 bits, 4 channels
      /5    mode 5, 44.1 kHz/16 bits, linear
      /6    mode 6, 44.1 kHz/16 bits, 'wide track' play

    DCC    DCC
      /A    Analogue transfer from media

    DVD    DVD
      /A    Analogue transfer from media

    TV     Television
      /PAL    PAL
      /NTSC   NTSC
      /SECAM  SECAM

    VID    Video
      /PAL    PAL
      /NTSC   NTSC
      /SECAM  SECAM
      /VHS    VHS
      /SVHS   S-VHS
      /BETA   BETAMAX

    RAD    Radio
      /FM   FM
      /AM   AM
      /LW   LW
      /MW   MW

    TEL    Telephone
      /I    ISDN

    MC     MC (normal cassette)
      /4    4.75 cm/s (normal speed for a two sided cassette)
      /9    9.5 cm/s
      /I    Type I cassette (ferric/normal)
      /II   Type II cassette (chrome)
      /III  Type III cassette (ferric chrome)
      /IV   Type IV cassette (metal)

    REE    Reel
      /9    9.5 cm/s
      /19   19 cm/s
      /38   38 cm/s
      /76   76 cm/s
      /I    Type I cassette (ferric/normal)
      /II   Type II cassette (chrome)
      /III  Type III cassette (ferric chrome)
      /IV   Type IV cassette (metal)

  TMOO
   The 'Mood' frame is intended to reflect the mood of the audio with a
   few keywords, e.g. "Romantic" or "Sad".
</code></pre>

</details>  

<details>

<summary>Rights and license frames</summary>

<pre><code>TCOP
   The 'Copyright message' frame, in which the string must begin with a
   year and a space character (making five characters), is intended for
   the copyright holder of the original sound, not the audio file
   itself. The absence of this frame means only that the copyright
   information is unavailable or has been removed, and must not be
   interpreted to mean that the audio is public domain. Every time this
   field is displayed the field must be preceded with "Copyright " (C) "
   ", where (C) is one character showing a C in a circle.

  TPRO
   The 'Produced notice' frame, in which the string must begin with a
   year and a space character (making five characters), is intended for
   the production copyright holder of the original sound, not the audio
   file itself. The absence of this frame means only that the production
   copyright information is unavailable or has been removed, and must
   not be interpreted to mean that the audio is public domain. Every
   time this field is displayed the field must be preceded with
   "Produced " (P) " ", where (P) is one character showing a P in a
   circle.

  TPUB
   The 'Publisher' frame simply contains the name of the label or
   publisher.

  TOWN
   The 'File owner/licensee' frame contains the name of the owner or
   licensee of the file and it's contents.

  TRSN
   The 'Internet radio station name' frame contains the name of the
   internet radio station from which the audio is streamed.

  TRSO
   The 'Internet radio station owner' frame contains the name of the
   owner of the internet radio station from which the audio is
   streamed.</code></pre>

</details>

<details>
<summary>Other text frames</summary>

<pre><code>TOFN
    The 'Original filename' frame contains the preferred filename for the
    file, since some media doesn't allow the desired length of the
    filename. The filename is case sensitive and includes its suffix.

TDLY
    The 'Playlist delay' defines the numbers of milliseconds of silence
    that should be inserted before this audio. The value zero indicates
    that this is a part of a multifile audio track that should be played
    continuously.
    
TDEN
    The 'Encoding time' frame contains a timestamp describing when the
    audio was encoded. Timestamp format is described in the ID3v2
    structure document [ID3v2-strct].
    
TDOR
    The 'Original release time' frame contains a timestamp describing
    when the original recording of the audio was released. Timestamp
    format is described in the ID3v2 structure document [ID3v2-strct].
    
TDRC
    The 'Recording time' frame contains a timestamp describing when the
    audio was recorded. Timestamp format is described in the ID3v2
    structure document [ID3v2-strct].
    
TDRL
    The 'Release time' frame contains a timestamp describing when the
    audio was first released. Timestamp format is described in the ID3v2
    structure document [ID3v2-strct].
    
TDTG
    The 'Tagging time' frame contains a timestamp describing then the
    audio was tagged. Timestamp format is described in the ID3v2
    structure document [ID3v2-strct].
    
TSSE
    The 'Software/Hardware and settings used for encoding' frame
    includes the used audio encoder and its settings when the file was
    encoded. Hardware refers to hardware encoders, not the computer on
    which a program was run.
    
TSOA
    The 'Album sort order' frame defines a string which should be used
    instead of the album name (TALB) for sorting purposes. E.g. an album
    named "A Soundtrack" might preferably be sorted as "Soundtrack".
    
TSOP
    The 'Performer sort order' frame defines a string which should be
    used instead of the performer (TPE2) for sorting purposes.
    
TSOT
    The 'Title sort order' frame defines a string which should be used
    instead of the title (TIT2) for sorting purposes.
</code></pre>

</details>

## TXXX frames (User defined text information frame)


## URL link frames

## WXXX frames

## COMM frames

## APIC frames

---
<div style="display: flex; justify-content: right;">
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>

