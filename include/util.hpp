//
// Created by dominik on 7/4/25.
//

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>

#include "config.hpp"

/**
 * Structure that holds information about a file like its path, name and extension.
 */
struct File_info {
    std::string path;      ///< path to the file
    std::string file_name; ///< name of the file
    std::string extension; ///< file extension
};
std::vector<File_info> parse_file_info(const std::vector<std::string>& input_files);

void print_mantis_help();
int compile_file(const File_info& fi, const Config& cfg);
bool support_ansi_color();
std::string demangle(const std::string& mangled_name);

/**
 * A wrapper for static_cast for nodes from e.g. the AST node. This makes debugging easier and centrally suppresses the
 * clang-tidy warning, since mantis makes use of handwritten checked downcasting for the nodes so no vtable needs to be
 * constructed which would increase memory cost.
 * @tparam To final type
 * @tparam From initial type
 * @param node object to type From that will be cast to To
 * @return cast pointer type
 */
template<typename To, typename From>
To* cast(From* node) {
    assert(node != nullptr && "casting nullptr");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<To*>(node);
}

#endif //UTIL_HPP
