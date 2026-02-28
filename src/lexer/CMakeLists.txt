# mantis/src/lexer/CMakeLists.txt

# collect all .cpp files in the lexer directory
file(GLOB LEXER_SOURCES *.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# compile the lexer into its own library
add_library(mantis_lexer STATIC ${LEXER_SOURCES})

# add include paths: both public and internal
target_include_directories(mantis_lexer
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# lexer depends on the util library
target_link_libraries(mantis_lexer PUBLIC mantis_util)
