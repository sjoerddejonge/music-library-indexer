---
layout: page
title: Music Library Indexer
---

<div style="display: flex; justify-content: space-between;">
    <span>
        <a href="{{ site.baseurl }}">{{ site.index_page_name }}</a> 
        <code>></code>  
        <a href="{{ site.baseurl }}/how_to_use.html">How to use</a>
    </span>
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>

---

# How to use

After installing `mli` to the `$PATH`, run it in the terminal in any 
directory of choice.

<img src="mli_terminal_example.gif" alt="Animation of a terminal window running the MLI app">

Available commands for `mli`:
```text
┌┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┬┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┐
┊           Music Library Indexer           ┊          Version: 0.1.0          ┊
└┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┴┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘

usage: mli <command> [<args>]

Supported commands:
index       Runs the Music Library Indexer in the current directory. Scans all
            (sub)directories for .aiff files, extracts id3 tag metadata, and
            writes to a JSON in the current/target directory.
            args:
            [<path>]    Full path to a directory to scan
            [-a]        Include APIC (attached picture) frame in JSON, with 
                        base64 encoding
            [-v]        Verbose console output
            [--shallow] Single directory only (no subdirectories)

help        Prints this text.
```

---
<div style="display: flex; justify-content: right;">
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>
