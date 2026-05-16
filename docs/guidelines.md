---
layout: page
title: Music Library Indexer
---

<div style="display: flex; justify-content: space-between;">
    <span>
        <a href="{{ site.baseurl }}">{{ site.index_page_name }}</a>
        <code>></code>  
        <a href="guidelines.md">Guidelines</a>
    </span>
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>

---

# Guidelines

Guidelines are a work-in-progress.

## Writing code
> Public functions in header files should use doxygen-style comments to
document the functionality.  

For example:
```c++
/**
 * @brief Extract all information from ID3 tag and write to JSON.
 * @param fin Reference to the ifstream of the music file
 * @param id3_pos The std::streampos of the ID3 tag in the music file
 * @param options A struct with options for running the command. For default see include/options.hpp
 * @return JSON with all parsed ID3 tag information of a song
 */
nlohmann::json id3ToJson(std::ifstream& fin, const std::streampos &id3_pos, const IndexOptions& options);
```

---
<div style="display: flex; justify-content: right;">
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>