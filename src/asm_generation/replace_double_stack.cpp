//
// Created by dominik on 7/22/25.
//

#include "asm_generator.hpp"

/**
 * Takes a list of instructions and checks if the instruction needs to be fixed up by replacing e.g. stack addresses
 * with registers. If that is the case, an auxiliary register (mostly R10) will be used in an additional instruction to
 * circumvent the use of two stack operands.
 * Also fixes idivs that opearate on a constant by first moving the constant to a register (R11)
 * @param insts list of instructions to fix
 */
void fix_instructions(std::vector<assem::Instruction>& insts) {
    for (size_t i=0; i<insts.size(); ++i) {
        std::visit([&]<class T0>(T0&& arg) {
            using T = std::decay_t<T0>;

            // mov instruction potentially needs fixing
            if constexpr (std::is_same_v<T, assem::Mov>) {
                // check if both operands are actually memory addresses
                const bool src_is_mem = std::holds_alternative<assem::Stack>(arg.src)
                                     or std::holds_alternative<assem::Data>(arg.src);
                const bool dst_is_mem = std::holds_alternative<assem::Stack>(arg.dst)
                                     or std::holds_alternative<assem::Data>(arg.dst);
                if (src_is_mem and dst_is_mem) {
                    // save original instruction
                    auto copy = arg;

                    // first, change source of original operator to R10
                    arg.src = assem::Register(assem::Reg::R10);

                    // next, insert move of source to scratch register R10 BEFORE the actual instruction
                    insts.insert(insts.begin() + i,
                            assem::Mov(copy.src, assem::Register(assem::Reg::R10)));
                }
            }

            // idiv might need fixing, as it cant operate on constants
            else if constexpr (std::is_same_v<T, assem::Idiv>) {
                // constant operand?
                if (std::holds_alternative<assem::Immediate_value>(arg.operand)) {
                    // save original instruction
                    auto copy = arg;

                    // set operand to r10
                    arg.operand = assem::Register(assem::Reg::R10);

                    // move constant to r10 before idiv
                    insts.insert(insts.begin() + i,
                            assem::Mov(copy.operand, assem::Register(assem::Reg::R10)));
                }
            }

            // cmp cant have two memory operands, and it cannot have a constant as the second operand
            else if constexpr (std::is_same_v<T, assem::Cmp>) {
                // check if both operands are actually memory addresses
                const bool lhs_is_mem = std::holds_alternative<assem::Stack>(arg.lhs)
                                     or std::holds_alternative<assem::Data>(arg.lhs);
                const bool rhs_is_mem = std::holds_alternative<assem::Stack>(arg.rhs)
                                     or std::holds_alternative<assem::Data>(arg.rhs);
                if (lhs_is_mem and rhs_is_mem) {
                    // save original instruction
                    auto copy = arg;
                    // first, change source of original operator to R10
                    arg.lhs = assem::Register(assem::Reg::R10);
                    // next, insert move of source to scratch register R10 BEFORE the actual instruction
                    insts.insert(insts.begin() + i,
                            assem::Mov(copy.lhs, assem::Register(assem::Reg::R10)));

                // constant as the rhs
                } else if (std::holds_alternative<assem::Immediate_value>(arg.rhs)) {
                    // save original instruction
                    auto copy = arg;
                    // first, use R11 as rhs in original instruction
                    arg.rhs = assem::Register(assem::Reg::R11);
                    // next, insert move of original rhs to R11 before instruction
                    insts.insert(insts.begin() + i,
                            assem::Mov(copy.rhs, assem::Register(assem::Reg::R11)));
                }
                return;
            }

            // some binary instruction potentially needs fixing
            // these currently are add, sub, and, or and xor
            else if constexpr (std::is_same_v<T, assem::Binary>) {
                // add and sub cant do memory-memory operations
                if (arg.op == assem::Bin_op::Add or arg.op == assem::Bin_op::Sub or
                    arg.op == assem::Bin_op::And or arg.op == assem::Bin_op::Or or arg.op == assem::Bin_op::Xor) {
                    // check if both operands are actually memory addresses
                    const bool src_is_mem = std::holds_alternative<assem::Stack>(arg.src)
                                         or std::holds_alternative<assem::Data>(arg.src);
                    const bool dst_is_mem = std::holds_alternative<assem::Stack>(arg.dst)
                                         or std::holds_alternative<assem::Data>(arg.dst);

                    if (src_is_mem and dst_is_mem) {
                        // save original instruction
                        auto copy = arg;

                        // first, change source of original operator to R10
                        arg.src = assem::Register(assem::Reg::R10);

                        // next, insert move of source to scratch register R10 BEFORE the actual instruction
                        insts.insert(insts.begin() + i,
                                assem::Mov(copy.src, assem::Register(assem::Reg::R10)));
                    }

                // imull can never have a memory destination
                } else if (arg.op == assem::Bin_op::Mult) {
                    // check if destination is in memory
                    const bool dst_is_mem = std::holds_alternative<assem::Stack>(arg.dst)
                                         or std::holds_alternative<assem::Data>(arg.dst);
                    if (dst_is_mem) {
                        // save original instruction
                        auto copy = arg;

                        // first, modify the instruction so that it uses r11 as destination
                        arg.dst = assem::Register(assem::Reg::R11);

                        // next, insert move from original dst to r11 before imull
                        insts.insert(insts.begin() + i,
                                assem::Mov(copy.dst, assem::Register(assem::Reg::R11)));

                        // lastly, insert move from r11 back to original destination
                        insts.insert(insts.begin() + i+2,
                                assem::Mov(assem::Register(assem::Reg::R11), copy.dst));

                    }
                }
            }

        }, insts[i]);
    }
}

/**
 * This function fixed up instructions in the assembly program that contain stack addresses as both operands, like e.g.
 * mov instructions. The resulting program will first move the source to an auxiliary register and then execute the
 * final instruction with this register as one of its operands.
 * @param prog program to fix up
 */
void Asm_generator::replace_double_stack(assem::Program& prog) {
    // loop over all top level definitions, only function definitions need to be handled
    for (auto& tp: prog) {
        std::visit([&]<class T0>(T0&& arg){
            using T = std::decay_t<T0>;

            // function found, now loop over all instructions and replace instructions with two stack operands
            if constexpr (std::is_same_v<T, assem::Function>) {
                fix_instructions(arg.insts);
            }
        }, tp);
    }
}
