//
// Created by dominik on 7/4/25.
//

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>

/**
 * Structure that holds information about a file like its path, name and extension.
 */
struct File_info {
    std::string path;      ///< path to the file
    std::string file_name; ///< name of the file
    std::string extension; ///< file extension
};

/**
 * Flags used by the compiler to control what stages should be executed or how much output should be given during the
 * compilation process.
 */
struct Compiler_flags {
    bool stop_after_parser;      ///< stops after parsing, no output file will be created
    bool stop_after_tac;         ///< stops after generation of the TAC tree, no output file will be created
    bool stop_after_codegen;     ///< stops after generation of the assembly tree, no output file will be created
    bool stop_after_compilation; ///< stops after the compilation itself, will emit the assembly code to a file
    bool stop_after_assembly;    ///< stops after assembling the code to an object file
    bool verbose;                ///< will give additional output information during compilation
    std::string arguments;       ///< string of command line arguments passed to the compiler
};

int parse_cli_arguments(int argc, const char* argv[],
                         std::vector<File_info>& file_info_vec, Compiler_flags& compiler_flags);
void print_mantis_help();
int compile_file(const File_info& fi, const Compiler_flags& cf);
bool support_ansi_color();
std::string demangle(const std::string& mangled_name);

/**
 * A wrapper for static_cast for nodes from e.g. the AST node. This makes debugging easier and centrally suppresses the
 * clang-tidy warning, since mantis makes use of handwritten checked downcasting for the nodes so no vtable needs to be
 * constructed which would increase memory cost.
 * @tparam To
 * @tparam From
 * @param node
 * @return
 */
template<typename To, typename From>
To* cast(From* node) {
    assert(node != nullptr && "casting nullptr");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<To*>(node);
}

#endif //UTIL_HPP
