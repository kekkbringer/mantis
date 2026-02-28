#include <filesystem>
#include <iostream>
#include <vector>

#include "version.hpp"
#include "util.hpp"

int main(const int argc, const char* argv[]) {
#ifdef DEBUG_MODE
    std::cout << "mantis " << MANTIS_VERSION << "\n";
    std::cout << "DEBUG MODE enabled\n";
    std::cout << "full call:\n";
    for (int i=0; i<argc; i++) std::cout << argv[i] << " ";
    std::cout << "\ncurrent path: " << std::filesystem::current_path() << "\n";
    std::cout << std::endl;
#endif

    // abort if no command line arguments were given
    if (argc < 2) {
        std::cerr << "Usage: mantis [options] <filename>\n";
        std::cerr << "use 'mantis --help' for more information\n";
        return 1;
    }

    int status;

    // structures for control flow
    std::vector<File_info> file_info_vec;
    Compiler_flags compiler_flags;

    const int cli_info = parse_cli_arguments(argc, argv, file_info_vec, compiler_flags);
    if (cli_info == -1) return 0; // '--help' or '--version' -> no compilation but successful termination
    if (cli_info != 0) return cli_info;

    // make call to compiler and assembler
    for (const auto& file_info: file_info_vec) {
        bool asm_file = false;

        // C-source files
        if (file_info.extension == ".c" or file_info.extension == ".cm") {
            status = compile_file(file_info, compiler_flags);
            if (status != 0) return status;

        // asm source files
        } else if (file_info.extension == ".s") {
            asm_file = true;
        }

        // call GCC assembler
        if (not compiler_flags.stop_after_compilation) {
            //const std::string asm_command =
            //    "gcc -c " + file_info.path + file_info.file_name
            //    + ".s -o " + file_info.path + file_info.file_name + ".o";
            const std::string asm_command =
                "gcc -c " + file_info.file_name + ".s -o " + file_info.file_name + ".o";

#ifdef DEBUG_MODE
            if (compiler_flags.verbose) {
                std::cout << asm_command << std::endl;
            }
#endif
            status = std::system(asm_command.c_str());
            if (status != 0) return status;

            // delete assembler file
            if (not compiler_flags.stop_after_compilation and not asm_file) {
                status = std::system(("rm -f " + file_info.file_name + ".s").c_str());
                if (status != 0) return status;
            }
        }
    }

    if (compiler_flags.stop_after_compilation) return 0;
    if (compiler_flags.stop_after_assembly) return 0;


    // link object files info final output
    std::string link_command = "gcc";
    //for (auto& fi: file_info_vec) link_command += " " + fi.path + fi.file_name + ".o";
    for (auto& fi: file_info_vec) link_command += " " + fi.file_name + ".o";
    //link_command += " -o " + file_info_vec[0].path + file_info_vec[0].file_name; // for legacy purposes
    link_command += " -o " + file_info_vec[0].file_name;

#ifdef DEBUG_MODE
    if (compiler_flags.verbose) {
        std::cout << link_command << std::endl;
    }
#endif
    status = std::system(link_command.c_str());
    if (status != 0) return status;


    // delete object files
    for (auto& fi: file_info_vec) {
        //std::system(("rm -f " + fi.path + fi.file_name + ".o").c_str());
        status = std::system(("rm -f " + fi.file_name + ".o").c_str());
        if (status != 0) return status;
    }

    return 0;
}