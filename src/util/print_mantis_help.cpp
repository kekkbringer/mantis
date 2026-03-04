//
// Created by dominik on 7/22/25.
//

#include "util.hpp"

#include <iostream>

/**
 * Prints the mantis help message.
 */
void print_mantis_help() {
    std::cout << "mantis [options] file\n";
    std::cout << "options:\n";
    std::cout << "  -h, --help             prints this message\n";
    std::cout << "  -v, --version          print version info\n";
    std::cout << "  -o, --output <file>    output is redirected to <file>\n";
    std::cout << "  -S, --emit-asm         only produces the assember code\n";
    std::cout << "  -c, --compile          only produces an object file\n";
    std::cout << "      --parse            stops after parsing the code\n";
    std::cout << "      --tac              stops after producing the TAC\n";
    std::cout << "      --codegen          stops after code generation\n";
    std::cout << "      --verbose          produces additional output\n";
    std::cout << "      --verbose-level <level> dictates how much output is given\n";
}
