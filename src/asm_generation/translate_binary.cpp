//
// Created by dominik on 7/12/25.
//

#include "asm_generator.hpp"

#include <cassert>

/**
 * Translates a general TAC binary instruction to the corresponding assembly instructions and appends them to a list of
 * assembly instructions.
 * @param ainsts list of assembly instructions
 * @param bp TAC binary instruction to be translated.
 */
void Asm_generator::translate_binary(std::vector<assem::Instruction>& ainsts, const tac::Binary* bp) {
    // first, translate lhs, rhs and destination of the binary instruction
    const auto lhs = translate_value(bp->lhs);
    const auto rhs = translate_value(bp->rhs);
    const auto dst = translate_value(bp->dst);

    // first special case: division or remainder
    // move lhs to AX, sign extend, idiv with rhs and move what's on AX to dst (div) or DX (mod)
    if (bp->op == tac::Binary_operator::Divide or bp->op == tac::Binary_operator::Remainder) {
        ainsts.emplace_back(assem::Mov(lhs, assem::Register(assem::Reg::AX)));
        ainsts.emplace_back(assem::Cdq());
        ainsts.emplace_back(assem::Idiv(rhs));

        bp->op == tac::Binary_operator::Divide ?
            ainsts.emplace_back(assem::Mov(assem::Register(assem::Reg::AX), dst))
          : ainsts.emplace_back(assem::Mov(assem::Register(assem::Reg::DX), dst));

        return;
    }

    // second special case: bitwise left/right shift SAL/SAR
    if (bp->op == tac::Binary_operator::Shift_left or bp->op == tac::Binary_operator::Shift_right) {
        // first, move lhs to destination
        ainsts.emplace_back(assem::Mov(lhs, dst));

        // translate operator
        // next, translate unary operator acting on the destination
        assem::Bin_op op;
        switch (bp->op) {
            case tac::Binary_operator::Shift_left: op = assem::Bin_op::Sal; break;
            case tac::Binary_operator::Shift_right: op = assem::Bin_op::Sar; break;
            default: assert(false && "in shift translation");
        }

        // if it is an immediate value just shift
        if (std::holds_alternative<assem::Immediate_value>(rhs)) {
            ainsts.emplace_back(assem::Binary(op, rhs, dst));

        // otherwise, if rhs is not an immediate value, move it to CX first
        } else {
            ainsts.emplace_back(assem::Mov(rhs, assem::Register(assem::Reg::CX)));
            ainsts.emplace_back(assem::Binary(op, assem::Register(assem::Reg::CX), dst));
        }

        return;
    }

    // special case: conditional operations
    if (bp->op == tac::Binary_operator::Equal        or bp->op == tac::Binary_operator::Not_equal or
        bp->op == tac::Binary_operator::Less_than    or bp->op == tac::Binary_operator::Less_or_equal or
        bp->op == tac::Binary_operator::Greater_than or bp->op == tac::Binary_operator::Greater_or_equal) {
        // 'compare' rhs with lhs in this order
        ainsts.emplace_back(assem::Cmp(rhs, lhs));

        // zero out destination, because conditional set will only
        ainsts.emplace_back(assem::Mov(assem::Immediate_value(0), dst));

        // set destination dependent on the operation
        assem::Cond_code cc;
        switch (bp->op) {
            case tac::Binary_operator::Equal: cc = assem::Cond_code::E; break;
            case tac::Binary_operator::Not_equal: cc = assem::Cond_code::NE; break;
            case tac::Binary_operator::Less_than: cc = assem::Cond_code::L; break;
            case tac::Binary_operator::Less_or_equal: cc = assem::Cond_code::LE; break;
            case tac::Binary_operator::Greater_than: cc = assem::Cond_code::G; break;
            case tac::Binary_operator::Greater_or_equal: cc = assem::Cond_code::GE; break;
            default: assert(false && "in conditional translation");
        }
        ainsts.emplace_back(assem::SetCC(cc, dst));
        return;
    }

    // general case: addition, subtraction, multiplication
    // and move the source to the destination in assembly
    ainsts.emplace_back(assem::Mov(lhs, dst));

    // next, translate unary operator acting on the destination
    assem::Bin_op op;
    switch (bp->op) {
        case tac::Binary_operator::Add: op = assem::Bin_op::Add; break;
        case tac::Binary_operator::Subtract: op = assem::Bin_op::Sub; break;
        case tac::Binary_operator::Multiply: op = assem::Bin_op::Mult; break;
        case tac::Binary_operator::Bit_and: op = assem::Bin_op::And; break;
        case tac::Binary_operator::Bit_or: op = assem::Bin_op::Or; break;
        case tac::Binary_operator::Bit_xor: op = assem::Bin_op::Xor; break;
        default: assert(false && "internal error in translate_binary");
    }

    ainsts.emplace_back(assem::Binary(op, rhs, dst));
}
