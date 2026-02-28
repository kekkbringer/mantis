//
// Created by dominik on 7/4/25.
//

#include "util.hpp"

#include <unistd.h>
#include <string>

/**
 * Helper function that determines whether the output stream supports the use of ANSI color codes.
 * @return true if output supports ANSI color codes
 */
bool support_ansi_color() {
    // terminal or not (maybe pipe into file)
    if (!isatty(STDOUT_FILENO)) return false;

    // read TERM environment variable to get more info
    const char* term = std::getenv("TERM");
    return term and std::string(term) != "dumb";
}