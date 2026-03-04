#include <filesystem>
#include <iostream>
#include <vector>

#include "version.hpp"
#include "cli_engine.hpp"
#include "config.hpp"
#include "util.hpp"
#include "version.hpp"

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
    Config config = parse_cli_arguments(argc, argv);

    if (config.print_version) {
        std::cout << "mantis " << MANTIS_VERSION << "\n";
        return 0;
    }

    if (config.print_help) {
        print_mantis_help();
        return 0;
    }

    const auto file_info_vec = parse_file_info(config.input_files);

    // make call to compiler and assembler
    for (const auto& file_info: file_info_vec) {
        bool asm_file = false;

        // C-source files
        if (file_info.extension == ".c" or file_info.extension == ".cm") {
            status = compile_file(file_info, config);
            if (status != 0) return status;

        // asm source files
        } else if (file_info.extension == ".s") {
            asm_file = true;
        }

        // call GCC assembler
        if (not config.stop_after_compilation) {
            //const std::string asm_command =
            //    "gcc -c " + file_info.path + file_info.file_name
            //    + ".s -o " + file_info.path + file_info.file_name + ".o";
            const std::string asm_command =
                "gcc -c " + file_info.file_name + ".s -o " + file_info.file_name + ".o";

#ifdef DEBUG_MODE
            if (config.verbose) {
                std::cout << asm_command << std::endl;
            }
#endif
            status = std::system(asm_command.c_str());
            if (status != 0) return status;

            // delete assembler file
            if (not config.stop_after_compilation and not asm_file) {
                status = std::system(("rm -f " + file_info.file_name + ".s").c_str());
                if (status != 0) return status;
            }
        }
    }

    if (config.stop_after_compilation) return 0;
    if (config.stop_after_assembly) return 0;


    // link object files info final output
    std::string link_command = "gcc";
    //for (auto& fi: file_info_vec) link_command += " " + fi.path + fi.file_name + ".o";
    for (auto& fi: file_info_vec) link_command += " " + fi.file_name + ".o";
    //link_command += " -o " + file_info_vec[0].path + file_info_vec[0].file_name; // for legacy purposes

    if (not config.output.empty()) {
        link_command += " -o " + config.output;
    } else {
        link_command += " -o " + file_info_vec[0].file_name;
    }

#ifdef DEBUG_MODE
    if (config.verbose) {
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
