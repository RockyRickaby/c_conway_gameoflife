# Game of Life

This is an implementation of Conway's Game of Life written in the C programming language.
My objective with this project was to learn how to use CMake, how to create a small library
using CMake (with the arena allocator implementation I've made) _and_ how to integrate libraries
into a project. Besides my own arena implementation, this project also uses Raylib to render
the game onto the screen.

Unlike my previous implementation of this, this one has a virtually infinite grid of cells.
I haven't tested it thoroughly, so I expect things to break under certain circumstances.

The allocations currently use arbitrary values. They might not matter much,
but I will make sure that lowering them doesn't break things... maybe.

https://github.com/user-attachments/assets/d7189701-ce46-4dcb-8218-b157c7625c62



## Building

To build this project, run the following commands. Don't forget to set the `CMAKE_BUILD_TYPE`
variable to `Debug` or `Release` in case you want specifically one build or the other.

```bash
$ cmake -B build # -D CMAKE_BUILD_TYPE=Debug # for generating the build files
$ cmake --build build # for building the project
```

Alternatively, you can always:

```bash
$ mkdir build && cd build
$ cmake ..
$ make
```

The presets all use Ninja (except for MSVC) instead of Make and only serve to enable compiler warnings.
Whether Make is faster than Ninja or not doesn't seem to matter too much when using the `-j` option
with Make (at least for this project, of course. I feel like Ninja is usually a bit faster).

```bash
$ cmake -B build --preset=clang # just an example. The preset can have a different name
```

I like Clang (for no reason, really), so I recommend the Clang preset :D.
