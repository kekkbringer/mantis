//
// Created by dominik on 7/11/25.
//

#include "asm_generator.hpp"

/**
 * Function to translate a TAC return instruction to an assembly instruction. First, the return value will be moved to
 * AX, then, control will be returned to the caller.
 * @param ainsts list of assembly instructions
 * @param rp TAC return instruction to be translated.
 */
void Asm_generator::translate_return(std::vector<assem::Instruction>& ainsts, const tac::Return* rp) {
    // get return value and move it to AX first
    const auto src = translate_value(rp->val);
    ainsts.emplace_back(assem::Mov(src, assem::Register(assem::Reg::AX)));

    // actual assembly return instruction
    ainsts.emplace_back(assem::Ret());
}
