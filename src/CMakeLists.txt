# mantis/src/CMakeLists.txt

# ensure correct build system is used
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# debugging the build type
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# compiler flags for debug and release build
set(CMAKE_CXX_FLAGS_DEBUG
    "-g -O0 -Wall -Wextra -Wpedantic -fsanitize=address -fsanitize=undefined -fsanitize=leak")
set(CMAKE_CXX_FLAGS_RELEASE
    "-O3 -DNDEBUG -march=native -flto=auto")

# for debug builds, define DEBUG_MODE
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_MODE)
endif()

# place final executable in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# add subdirectories for each module
add_subdirectory(parser)
add_subdirectory(lexer)
add_subdirectory(util)
add_subdirectory(tac_generation)
add_subdirectory(asm_generation)
add_subdirectory(code_emission)

add_executable(mantis main.cpp)
target_link_libraries(mantis
    mantis_lexer
    mantis_tac
    mantis_asm
    mantis_code_emission
    mantis_parser
    mantis_util
)
