# Modding

This game was made with extensibility and modding as a core feature. The game engine and game logic are separated into the executable and shared libraries. The game engine source files are located in `src`, and the game logic source files are located in `plugins`. At compile time, I plan to implement the build system so that it will look for directories in the `plugin` directory, compile all the source files in that directory, and create a shared library in the `bin` directory. At runtime, the game engine will look for a `plugins` directory in the current directory. In the `plugins` directory, the engine will try to load a shared library. This is demonstrated with the base plugin `soultaker`.

The base game will have an interface to load mods. If you're familiar with factorio, I'm going for that vibe.
