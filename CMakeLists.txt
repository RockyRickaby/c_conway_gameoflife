cmake_minimum_required(VERSION 4.0)
project(game_of_life_c VERSION 0 LANGUAGES C)

set(needed_features c_std_11)

add_executable(main
	src/main.c
  src/set.c
  src/gol.c
)

set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
  endif()
endif()

find_package(arena QUIET)
if (NOT arena_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    arena
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    GIT_REPOSITORY https://github.com/RockyRickaby/simple_arena.git
    GIT_TAG 9ad3483 # this is annoying to keep track of...
  )
  FetchContent_GetProperties(arena)
  if (NOT arena_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(arena)
  endif()
endif()

target_link_libraries(main raylib arena)
target_compile_features(main PUBLIC ${needed_features})