# Game of Life

This is an implementation of Conway's Game of Life written in the C programming language.
My objective with this project was to learn how to use CMake, how to create a small library
using CMake (with the arena allocator implementation I've made) _and_ how to integrate libraries
into a project. Besides my own arena implementation, this project also uses Raylib to render
the game onto the screen.

## Building

> [!WARNING]
> This project has only been tested on Linux. The implementation isn't particularly
> platform specific, but the generation step hasn't been tested on other platforms.

To build this project, run the following commands. Don't forget to set the `CMAKE_BUILD_TYPE`
variable to `Debug` or `Release` in case you want specifically one or the other.

```bash
$ cmake -B build
$ cmake --build build 
```

Alternatively, you can always:

```bash
$ mkdir build && cd build
$ cmake ..
$ make
```

The presets all use Ninja (except for MSVC) instead of Make and only serve to enable compiler warnings.
I recommend Ninja over Make because it often seems to build projects like Raylib
a lot faster than Make. In case you use Ninja, the last command listed above
should be changed to simply `ninja`. A preset can be defined, for example, as follows:

```bash
$ cmake -B build --preset=Clang # just an example. The preset can have a different name
```

I like Clang (for no reason, really), so I recommend the Clang preset :D.