---
layout: page
title: Music Library Indexer
---

<div style="display: flex; justify-content: space-between;">
    <span>
        <a href="{{ site.baseurl }}">{{ site.index_page_name }}</a>
        <code>></code>  
        <a href="{{ site.baseurl }}/guidelines.html">Guidelines</a>
    </span>
    <a href="{{ site.repo_url }}">{{ site.return_text }}</a>
</div>

---

# Guidelines

Guidelines are a work-in-progress.


## Commits
All commits should have one of the following types:

* `feat`: Adds a new feature.
* `fix`: Fixes a bug.
* `refactor`: Rewrites/restructures code and does not add features or
  fixes bugs.
* `chore`: Changes that are not related to a fix or feature and do not
  modify code in src or test files. Examples: Changing version number,
  modifying .gitignore file, updating dependencies.
* `docs`: Updates the documentation, README.md
* `style`: Changes that do not affect the meaning of the code, likely
  related to code formatting such as white-space, missing semi-colons, etc.
* `revert`: Revert a previous commit.
* `test`: Add or correct test(s).


## Writing code
> Public functions in header files should use doxygen-style comments to
document the functionality.  

The comment should document what the function does, what each parameter
represents, and what it returns:
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