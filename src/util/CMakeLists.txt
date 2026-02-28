# mantis/src/util/CMakeLists.txt

# collect all .cpp files in the util directory
file(GLOB UTIL_SOURCES *.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# compile all utilities into their own library
add_library(mantis_util STATIC ${UTIL_SOURCES}
        util.cpp)


target_include_directories(mantis_util
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# util dependencies
target_link_libraries(mantis_util PUBLIC mantis_code_emission)
