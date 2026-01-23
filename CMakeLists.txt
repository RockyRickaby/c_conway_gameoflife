cmake_minimum_required(VERSION 4.0)
project(game_of_life_c VERSION 0 LANGUAGES C)

# include(CheckIPOSupported)
# check_ipo_supported(RESULT lto_supported OUTPUT error)

set(needed_features c_std_11)

add_executable(main
	src/main.c
  src/set.c
  src/gameoflife.c
)

# might be useless
# if (lto_supported)
#   set_target_properties(main PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
# endif()

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
    GIT_TAG f5303a0 # this is annoying to keep track of...
  )
  FetchContent_GetProperties(arena)
  if (NOT arena_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(arena)
  endif()
endif()

target_link_libraries(main raylib arena)
target_compile_features(main PUBLIC ${needed_features})