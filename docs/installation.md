---
layout: page
title: Music Library Indexer
---

<div style="display: flex; justify-content: space-between;">
    <span>
        <a href="{{ site.baseurl }}">Home</a> <code>></code>
        <a href="installation.md">Installation</a>
    </span>
    <a href="{{ site.repo_url }}"> ← Return to repo </a>
</div>

---

# Installation
The Music Library Indexer is a CLI app that can be added to the `$PATH`
environment variable. This allows the user to call the app anywhere in
the terminal by using the binary name (default is `mli`).

## For macOS
Download the [latest release](https://github.com/sjoerddejonge/music-library-indexer/releases/latest/download/mli-macos-arm64),
rename the file to `mli`, and move it to a directory included in the `$PATH`.
On macOS, the (hidden) directory `/usr/local/bin/` is a good option, but
the installation requires admin privileges.

Alternatively, you could run the following script in your terminal:

```commandline
curl -L https://github.com/sjoerddejonge/music-library-indexer/releases/latest/download/mli-macos-arm64 -o mli
sudo mv mli /usr/local/bin/mli
chmod +x /usr/local/bin/mli
```

## For Linux
Soon!

## For Windows
* Download the [latest release for Windows](https://github.com/sjoerddejonge/music-library-indexer/releases/latest/download/mli-windows-x86_64.exe) (for ARM you have to build it 
yourself)

* Add the file to a folder, `C:\Users\username\bin` for example

* Rename it to just `mli.exe`

* Add that folder to your `$PATH`:
  * Search for `env` in the Start menu
  * Click "Environment Variables"
  * Edit the "Path" variable
  * Add the folder you copied `mli.exe` to

---
<div style="display: flex; justify-content: right;">
    <a href="{{ site.repo_url }}"> ← Return to repo </a>
</div>
