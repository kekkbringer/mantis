# mantis/CMakeLists.txt

cmake_minimum_required(VERSION 3.19)
project(mantis)

# define version components
set(mantis_VERSION_MAJOR 0)
set(mantis_VERSION_MINOR 1)
set(mantis_VERSION_PATCH 0)
set(mantis_VERSION "${mantis_VERSION_MAJOR}.${mantis_VERSION_MINOR}.${mantis_VERSION_PATCH}")

# use C++ 23 standard
set(CMAKE_CXX_STANDARD 23)

# add source subdirectory
add_subdirectory(src)

# configure version number header
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/version.hpp.in"
    "${CMAKE_BINARY_DIR}/version.hpp"
)

# link version header into project
target_include_directories(mantis PUBLIC ${CMAKE_BINARY_DIR})
target_include_directories(mantis_util PUBLIC ${CMAKE_BINARY_DIR})

# test section
enable_testing()
add_subdirectory(tests)


# Doxygen
find_package(Doxygen)

if (DOXYGEN_FOUND)
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM
	)
endif()
