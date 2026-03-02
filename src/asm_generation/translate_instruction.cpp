//
// Created by dominik on 7/11/25.
//

#include "asm_generator.hpp"
#include "util.hpp"

#include <cassert>

/**
 * This function takes a list of TAC instructions, translates them to their corresponding assembly instructions
 * nodes and appends them to a list.
 * @param ainsts list of assembly instructions
 * @param tinsts list of TAC instructions to be translated
 */
void Asm_generator::translate_instructions(std::vector<assem::Instruction>& ainsts, const std::span<tac::Inst*>& tinsts) {
    for (const auto& tinst: tinsts) {
        switch (tinst->kind) {
            case tac::Inst::Kind::Return: {
                const auto* ret = cast<tac::Return const>(tinst);
                translate_return(ainsts, ret);
                break;
            }

            case tac::Inst::Kind::Unary: {
                const auto* un = cast<tac::Unary const>(tinst);
                translate_unary(ainsts, un);
                break;
            }

            case tac::Inst::Kind::Binary: {
                const auto* bi = cast<tac::Binary const>(tinst);
                translate_binary(ainsts, bi);
                break;
            }

            case tac::Inst::Kind::Jump_if_zero: {
                const auto* jiz = cast<tac::Jump_if_zero const>(tinst);
                const auto cond = translate_value(jiz->cond);
                ainsts.emplace_back(assem::Cmp(assem::Immediate_value(0), cond));
                ainsts.emplace_back(assem::JmpCC(assem::Cond_code::E, jiz->target.data()));
                break;
            }

            case tac::Inst::Kind::Jump_if_not_zero: {
                const auto* jinz = cast<tac::Jump_if_not_zero const>(tinst);
                auto cond = translate_value(jinz->cond);
                ainsts.emplace_back(assem::Cmp(assem::Immediate_value(0), cond));
                ainsts.emplace_back(assem::JmpCC(assem::Cond_code::NE, jinz->target.data()));
                break;
            }

            case tac::Inst::Kind::Jump: {
                const auto* jump = cast<tac::Jump const>(tinst);
                ainsts.emplace_back(assem::Jmp(jump->target.data()));
                break;
            }

            case tac::Inst::Kind::Label: {
                const auto* lab = cast<tac::Label const>(tinst);
                ainsts.emplace_back(assem::Label(lab->name.data()));
                break;
            }

            case tac::Inst::Kind::Copy: {
                const auto* cpy = cast<tac::Copy const>(tinst);
                auto src = translate_value(cpy->src);
                auto dst = translate_value(cpy->dst);
                ainsts.emplace_back(assem::Mov(src, dst));
                break;
            }

            case tac::Inst::Kind::Function_call: {
                const auto* fc = cast<tac::Function_call const>(tinst);
                translate_function_call(fc, ainsts);
                break;
            }

            default:
                assert(false && "translate tac instruction");
                std::unreachable();
        }
    }
}
