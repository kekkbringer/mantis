find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/valid/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/invalid/*.c"
)

foreach(test_file ${TEST_FILES})
    file(READ "${test_file}" CONTENTS)

    # parse tags from file (optional)
    set(FILE_TAGS "")
    string(REGEX MATCH "//[ ]*tags?[ ]*:[ ]*([^\n]*)" _ "${CONTENTS}")
    if(CMAKE_MATCH_1)
        set(TAG_STRING "${CMAKE_MATCH_1}")
        string(REPLACE "," ";" FILE_TAGS "${TAG_STRING}")
    endif()

    # get relative path to tests/ directory (strip prefix up to "tests/")
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${test_file}")
    # rel_path example: valid/simple/test1.c

    # extract directories from the relative path (remove filename)
    get_filename_component(dir_path "${rel_path}" DIRECTORY)
    # dir_path example: valid/simple

    # split directory path into tags (split by /)
    string(REPLACE "/" ";" DIR_TAGS "${dir_path}")

    # combine file tags and directory tags, avoid duplicates
    set(ALL_TAGS "")
    foreach(t ${FILE_TAGS})
        string(STRIP "${t}" t)
        list(APPEND ALL_TAGS "${t}")
    endforeach()
    foreach(t ${DIR_TAGS})
        string(STRIP "${t}" t)
        # add only if not already present
        list(FIND ALL_TAGS "${t}" index)
        if(index EQUAL -1 AND NOT t STREQUAL "")
            list(APPEND ALL_TAGS "${t}")
        endif()
    endforeach()

    # generate safe test name (replace / with _)
    string(REPLACE "/" "_" safe_test_name "${rel_path}")

    # register the test
    add_test(NAME "test_${safe_test_name}"
            COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/run_test.py"
            "${test_file}" "${CMAKE_BINARY_DIR}/bin/mantis")

    # set combined labels if any
    if(ALL_TAGS)
        set_tests_properties("test_${safe_test_name}" PROPERTIES LABELS "${ALL_TAGS}")
    endif()
endforeach()
