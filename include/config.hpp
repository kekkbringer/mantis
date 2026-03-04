//
// Created by dominik on 3/4/26.
//

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

/**
 * Flags used by mantis to control what stages should be executed or how much output should be given during the
 * compilation process.
 */
struct Config {
    std::vector<std::string> input_files; ///< vector containg the path of all input files
    bool stop_after_parser = false;       ///< stops after parsing, no output file will be created
    bool stop_after_tac = false;          ///< stops after generation of the TAC tree, no output file will be created
    bool stop_after_codegen = false;      ///< stops after generation of the assembly tree, no output file will be created
    bool stop_after_compilation = false;  ///< stops after the compilation itself, will emit the assembly code to a file
    bool stop_after_assembly = false;     ///< stops after assembling the code to an object file
    bool verbose = false;                 ///< will give additional output information during compilation
    int verbose_level = 1;                ///< set level of verbosity
    std::string output;                   ///< path of output file
    bool print_version = false;           ///< only print version info
    bool print_help = false;              ///< only print help message
};


#endif //CONFIG_HPP
