//
// Created by dominik on 7/11/25.
//

#include "asm_generator.hpp"

/**
 * Main dirver routine of the translation process from the TAC tree to the assemly tree. The translation process is
 * conducted in different passes, there are:
 *
 * In the first pass, the main part of the translation is carried out and already results in an assembly program tree.
 * This program is not yet valid, as it contains pseudo register used in the translation process and might also include
 * illegal operands in instructions (e.g. a mov instruction that takes two stack addresses).
 *
 * In the second pass, all pseudo register are replace by address on the stack. Additionally, the total stack space of
 * each function is calculated and set to be allocated in the code emission step later.
 *
 * In the third pass, illegal instruction calls are fix. One example are mov instructions that have stack addresses as
 * both operands. These instructions are then modified and new instructions are inserted into the list, that e.g. first
 * move one of the operands into an auxiliary register.
 * @return
 */
assem::Program Asm_generator::gen() {
    assem::Program prog;

    // translate TAC program to (most probably invalid) assembler program
    translate_program(prog);

    // replace pseudo register with stack addresses
    replace_pseudo_regs(prog);

    // replace instructions that contain two stack addresses
    replace_double_stack(prog);

    return prog;
}