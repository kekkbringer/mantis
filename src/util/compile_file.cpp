//
// Created by dominik on 7/12/25.
//

#include "util.hpp"

#include "arena_allocator.hpp"
#include "string_table.hpp"
#include "source_manager.hpp"
#include "diagnostics_engine.hpp"
#include "parser.hpp"
#include "symbol_table.hpp"
#include "tac_generator.hpp"
#include "asm_generator.hpp"
#include "code_emitter.hpp"

/**
 * Drives the compilation of the file given by the File_info. The Compiler flags control the details of this process
 * like early termination or detailed output.
 * First, the GCC preprocessor is called. Second, the mantis parser generated the abstract syntax tree (AST). Then, the
 * AST is translated to a three address code (TAC). Next, the TAC is translated to assembly which is then emitted to a
 * file in the final step of compilation.
 * @param fi contains information of the file that's to be compiled
 * @param cf contains flags used during the compilation process
 * @return status code, 0 only if everything was successful
 */
int compile_file(const File_info& fi, const Compiler_flags& cf) {
    // if .s file is given -> just go to assembler
    if (fi.extension == ".s" or fi.extension == ".S") return 0;


    // mantis compiler call

    // setup source code
    Source_manager source_manager(fi);
    if (const int status = source_manager.status(); status != 0) return status;

    // setup diagnostics engine
    Diagnostics_engine diag_engine(source_manager);

    // setup symbol table
    Scope file_scope(nullptr);

    // setup arena allocator and string table for the AST
    ArenaAllocator arena;
    StringTable string_table;

    // call the parser
    Parser parser(source_manager, diag_engine, &file_scope, &string_table, &arena);
    auto prog = parser.parse();
    if (const int status = diag_engine.status(); status != 0) return status;
#ifdef DEBUG_MODE
    std::cout << "\n\nprinting AST program:\n";
    Parser::print_program(prog);
#endif
    if (cf.stop_after_parser) return 0;

    // generate TAC program
    Tac_generator tac_gen(prog, &file_scope);
    auto tac_prog = tac_gen.gen();
#ifdef DEBUG_MODE
    std::cout << "\n\nprinting TAC program:\n";
    Tac_generator::print_tac(tac_prog);
#endif
    if (cf.stop_after_tac) return 0;

    // translate TAC to assembler
    Asm_generator asm_gen(tac_prog, &file_scope);
    auto asm_prog = asm_gen.gen();
#ifdef DEBUG_MODE
    std::cout << "\n\nprinting ASM program:\n";
    Asm_generator::print_asm(asm_prog);
#endif
    if (cf.stop_after_codegen) return 0;

    // code emission
    //const std::string asm_file = (fi.path + fi.file_name + ".s");
    const std::string asm_file = (fi.file_name + ".s");
    std::ofstream outfile(asm_file);
    Code_emitter ce(outfile, asm_prog);
    ce.emit_code();
    outfile.close();
    return 0;
}
