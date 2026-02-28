//
// Created by dominik on 7/12/25.
//

#ifndef CODE_EMITTER_HPP
#define CODE_EMITTER_HPP

#include "asm_nodes.hpp"

#include <iostream>

/**
 * The Code_emitter can be initialized with an assembly program tree and an optional ostream. If no ostream is given,
 * std::cout will be chosen.
 *
 * The code emitter can then be used to emit the assembly program tree as legal assembly code to the stream specified
 * in the initialization of the emitter.
 *
 * Currently, will include some comments in the final assembly source code.
 */
class Code_emitter {
private:
    assem::Program prog; ///< assembly program to be emitted as code
    std::ostream& os;    ///< stream that the code is emitted to (std::cout if not further specified)

    void emit_top_level(const assem::Top_level& tp);
    void emit_function_definition(const assem::Function& f);
    void emit_static_variable(const assem::Static_variable& v) const;
    void emit_instruction(const assem::Instruction& i);
    void emit_operand(const assem::Operand& o, int size = 4);

public:
    Code_emitter(std::ostream& os, assem::Program& prog) : prog(std::move(prog)), os(os) {}
    explicit Code_emitter(assem::Program& prog) : prog(std::move(prog)), os(std::cout) {}

    void emit_code();
};

#endif //CODE_EMITTER_HPP
