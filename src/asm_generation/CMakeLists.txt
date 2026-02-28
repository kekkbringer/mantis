# mantis/src/asm_generation/CMakeLists.txt

# collect all .cpp files in the asm generation directory
file(GLOB ASM_SOURCES *.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# compile the asm sources into its own library
add_library(mantis_asm STATIC ${ASM_SOURCES})

# add include paths: both public and internal
target_include_directories(mantis_asm
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# asm dependencies
target_link_libraries(mantis_asm PUBLIC mantis_util mantis_tac)
