//
// Created by dominik on 7/12/25.
//

#include "code_emitter.hpp"

/**
 * Emits the source code for an assembly function definition by first, emitting the function prolog where %rsp and %rbp
 * will be set and needed stack space is allocated. Then, all instructions in the function are traversed and emitted.
 * Lastly, when emitting the return instruction, the function epilog takes places, where %rsp and %rbp are restored to
 * their previous values.
 * @param f assembly function definition node
 */
void Code_emitter::emit_function_definition(const assem::Function& f) {
    // emit text label
    os << "\t.text\n";

    // maybe emit global label of function
    if (f.global) {
        os << "\t.globl " << f.name << "\n";
        os << f.name << ":\n";
    }

#ifdef DEBUG_MODE
    os << "# function prolog\n";
#endif
    os << "\tpushq\t%rbp\n";
    os << "\tmovq\t%rsp, %rbp\n";
#ifdef DEBUG_MODE
    os << "\n";
#endif

    // emit instructions of function body
    for (const auto& i: f.insts) {
        emit_instruction(i);
    }
}
