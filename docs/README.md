# Overview

Soultaker is a roguelike bullet-hell with mechanics inspired by Realm of the Mad God. \
This directory contains documentation about the game engine and game mechanics.

Here is a high level overview of the file structure:
- `assets` contains the assets. This is not strictly enforced, as these assets are pointed to by paths in config files, but they should all be here.
- `config` contains the configuration files. This directory must be present and must contain some more directories, which will be recursively searched for configuration files. These config files are merged as they are loaded. The configuration files all use json. My json parser can't discern duplicate keys (yet), and duplicate keys will cause undefined behavior, so avoid those. 
    - `entities` contains configuration files for entities
    - `items` contains configuration files for items
    - `maps` contains configuration files for maps
    - `textures` contains configuration files for textures
- `src` contains all of the source files for the engine.
- `plugins` contains all of the source files for the plugins.
- `lib` and `include` contain dependencies, which are `glfw`, `glad`, `miniaudio`, and `stb`
    - more dependencies that I programmed are in `src/util`
- `build` contains object files that are creating during the building process (after running `make`)
- `bin` contains the executables


For information about the differences between `src` and `plugins`, see `modding.md`

