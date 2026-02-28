# mantis/src/code_emission/CMakeLists.txt

# collect all .cpp files in the code emission directory
file(GLOB CE_SOURCES *.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# compile the code emission sources into its own library
add_library(mantis_code_emission STATIC ${CE_SOURCES}
        emit_static_variable.cpp)

# add include paths: both public and internal
target_include_directories(mantis_code_emission
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# code emission dependencies
target_link_libraries(mantis_code_emission PUBLIC mantis_util mantis_asm)
