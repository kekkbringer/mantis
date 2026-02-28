//
// Created by dominik on 7/22/25.
//

#include <iostream>
#include <sys/stat.h>

#include "util.hpp"
#include "version.hpp"

/**
 * Parses the command line arguments that were passed to mantis, currently supported arguments are:
 *  - --help, will print the mantis help message
 *  - --parser, will only run the parser and terminate afterwards
 *  - --tac, will only generate the TAC tree and terminate afterwards
 *  - --codegen, will stop after generation of the assembly tree but not emit it
 *  - -S, will stop after compilation, emitting assembly to a file
 *  - -c, will stop after assembly, emitting an object file
 *  - --verbose, will give additional output during compilation process
 *  - --version, will print the version of mantis
 *  - <file list>, list of files to be compiled
 *
 *  If '--help' or '--version' was found, the function returns -1 to indicate that no compilation should be done.
 * @param argc total number of command line arguments
 * @param argv holds the command line arguments
 * @param file_info_vec holds information about the files passed to mantis
 * @param compiler_flags controls details of compilation process
 * @return status code, 0 only if everything was successful
 */
int parse_cli_arguments(const int argc, const char* argv[],
                        std::vector<File_info>& file_info_vec, Compiler_flags& compiler_flags) {
    compiler_flags.stop_after_parser = false;
    compiler_flags.stop_after_tac = false;
    compiler_flags.stop_after_codegen = false;
    compiler_flags.stop_after_compilation = false;
    compiler_flags.stop_after_assembly = false;
    compiler_flags.verbose = false;

    // loop over command line arguments
    for (int i=1; i < argc; i++) {
        const std::string arg = argv[i];

        if (arg == "--help") {
            print_mantis_help();
            return -1;

        } else if (arg == "--parser") {
            compiler_flags.stop_after_parser = true;

        } else if (arg == "--tac") {
            compiler_flags.stop_after_tac = true;

        } else if (arg == "--codegen") {
            compiler_flags.stop_after_codegen = true;

        } else if (arg == "-S") {
            compiler_flags.stop_after_compilation = true;

        } else if (arg == "-c") {
            compiler_flags.stop_after_assembly = true;

        } else if (arg == "--verbose") {
            compiler_flags.verbose = true;

        } else if (arg == "--version" or arg == "-v") {
            std::cout << "mantis " << MANTIS_VERSION << "\n";
            return -1;

        } else if (arg.substr(0, 1) == "-") {
            std::cout << "Unknown option " << arg << "\n";
            return 1;

        // if not a command line option, expect a file
        } else {
            File_info info;

            // find last occurrences of '/' and '.' to extract path, file name and extension
            const auto foundSlash = arg.find_last_of('/');
            const auto foundDot   = arg.find_last_of('.');
            if (foundSlash == std::string::npos) {
                info.path = "";
            } else {
                info.path = arg.substr(0, foundSlash+1);
            }
            if (foundDot == std::string::npos) {
                info.extension = "";
            } else {
                info.extension = arg.substr(foundDot);
            }

            info.file_name = arg.substr(foundSlash+1, foundDot-foundSlash-1);

            if (info.file_name.empty()) {
                std::cout << "Invalid file name: " << arg << std::endl;
                return 1;
            }

            if (info.extension != ".c" and info.extension != ".s" and info.extension != ".S") {
                std::cout << "Warning: Unknow file extension: " << info.extension << "\n";
                std::cout << "Treating it as a C-source file." << std::endl;
            }

            // check if the file actually exists
            struct stat buffer;
            if (stat(arg.c_str(), &buffer) != 0) {
                std::cout << "Could not find file: " << arg << std::endl;
                return 1;
            }

            file_info_vec.push_back(info);
        }
    }
#ifdef DEBUG_MODE
    if (compiler_flags.verbose) {
        for (const auto&[path, file_name, extension]: file_info_vec) {
            std::cout << "extracted file information as\n"
                << "   path: " << path << "\n"
                << "   file_name: " << file_name << "\n"
                << "   file extension: " << extension << "\n\n" << std::flush;
        }
    }
#endif

    return 0;
}
