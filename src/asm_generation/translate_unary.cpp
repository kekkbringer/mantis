//
// Created by dominik on 7/12/25.
//

#include "asm_generator.hpp"

#include <cassert>

/**
 * Translates a general TAC unary instruction to the corresponding assembly instructions and appends them to a list of
 * assembly instructions.
 * @param ainsts list of assembly instructions
 * @param up TAC unary instruction to be translated.
 */
void Asm_generator::translate_unary(std::vector<assem::Instruction>& ainsts, const tac::Unary_ptr& up) {
    const auto src = translate_value(up->src);
    const auto dst = translate_value(up->dst);

    // special case: logical not
    if (up->op == tac::Unary_operator::Not) {
        ainsts.emplace_back(assem::Cmp(assem::Immediate_value(0), src));
        ainsts.emplace_back(assem::Mov(assem::Immediate_value(0), dst));
        ainsts.emplace_back(assem::SetCC(assem::Cond_code::E, dst));

        return;
    }

    // first, translate source and destination of the unary instruction
    // and move the source to the destination in assembly
    ainsts.emplace_back(assem::Mov(src, dst));

    // next, translate unary operator acting on the destination
    assem::Un_op op;
    switch (up->op) {
        case tac::Unary_operator::Complement: op = assem::Un_op::Not; break;
        case tac::Unary_operator::Negate: op = assem::Un_op::Neg; break;
        default: assert(false && "internal error in translate_unary");
    }

    ainsts.emplace_back(assem::Unary(op, dst));
}
