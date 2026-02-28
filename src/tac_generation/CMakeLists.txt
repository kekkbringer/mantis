# mantis/src/tac_generation/CMakeLists.txt

# collect all .cpp files in the tac generation directory
file(GLOB TAC_SOURCES *.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# compile the tac sources into its own library
add_library(mantis_tac STATIC ${TAC_SOURCES}
        traverse_scope.cpp)

# add include paths: both public and internal
target_include_directories(mantis_tac
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# parser dependencies
target_link_libraries(mantis_tac PUBLIC mantis_util mantis_parser)
