# mantis/src/parser/CMakeLists.txt

# collect all .cpp files in the parser directory
file(GLOB PARSER_SOURCES *.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# compile the parser into its own library
add_library(mantis_parser STATIC ${PARSER_SOURCES})

# add include paths: both public and internal
target_include_directories(mantis_parser
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# parser dependencies
target_link_libraries(mantis_parser PUBLIC mantis_util mantis_lexer)
