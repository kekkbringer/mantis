//
// Created by dominik on 7/11/25.
//

#include "asm_generator.hpp"

#include <cassert>

/**
 * This function takes a list of TAC instructions, translates them to their corresponding assembly instructions
 * nodes and appends them to a list.
 * @param ainsts list of assembly instructions
 * @param tinsts list of TAC instructions to be translated
 */
void Asm_generator::translate_instructions(std::vector<assem::Instruction>& ainsts, const std::vector<tac::Instruction_ptr>& tinsts) {
    for (const auto& tinst: tinsts) {
        std::visit([&]<class T0>(T0&& arg) {
            using T = std::decay_t<T0>;

            // return statement
            if constexpr (std::is_same_v<T, tac::Return_ptr>) {
                translate_return(ainsts, arg);

            // unary operation
            } else if constexpr (std::is_same_v<T, tac::Unary_ptr>) {
                translate_unary(ainsts, arg);

            // binary operation
            } else if constexpr (std::is_same_v<T, tac::Binary_ptr>) {
                translate_binary(ainsts, arg);

            // jump if zero
            } else if constexpr (std::is_same_v<T, tac::Jump_if_zero_ptr>) {
                auto cond = translate_value(arg->cond);
                ainsts.emplace_back(assem::Cmp(assem::Immediate_value(0), cond));
                ainsts.emplace_back(assem::JmpCC(assem::Cond_code::E, arg->target));

            // jump if not zero
            } else if constexpr (std::is_same_v<T, tac::Jump_if_not_zero_ptr>) {
                auto cond = translate_value(arg->cond);
                ainsts.emplace_back(assem::Cmp(assem::Immediate_value(0), cond));
                ainsts.emplace_back(assem::JmpCC(assem::Cond_code::NE, arg->target));

            // jump
            } else if constexpr (std::is_same_v<T, tac::Jump_ptr>) {
                ainsts.emplace_back(assem::Jmp(arg->target));

            // label
            } else if constexpr (std::is_same_v<T, tac::Label_ptr>) {
                ainsts.emplace_back(assem::Label(arg->name));

            // copy
            } else if constexpr (std::is_same_v<T, tac::Copy_ptr>) {
                auto src = translate_value(arg->src);
                auto dst = translate_value(arg->dst);
                ainsts.emplace_back(assem::Mov(src, dst));

            // function call
            } else if constexpr (std::is_same_v<T, tac::Function_call_ptr>) {
                translate_function_call(arg, ainsts);

            // unhandled TAC instruction
            } else {
                assert(false && "unhandled case in translate_instructions loop");
            }
        }, tinst);
    }
}
