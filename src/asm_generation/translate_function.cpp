//
// Created by dominik on 7/11/25.
//

#include "asm_generator.hpp"

#include <iostream>
#include <array>

/**
 * This function translates a TAC function definition to an assembly function definition.
 * @param fp pointer to the function that will be translated
 * @return translated assembly function node
 */
assem::Function Asm_generator::translate_function(const tac::Function_ptr& fp) {
    if (fp == nullptr) return {};

    assem::Function func;

    // keep name and scoping
    func.name = fp->name;
    func.global = fp->global;

    // get parameters from registers
    constexpr std::array<const assem::Register, 6> param_registers = {reg::DI, reg::SI, reg::DX, reg::CX, reg::R8, reg::R9};
    int p_counter = 0;
    for (const auto& param: fp->params) {
        const assem::Operand asm_param = assem::Pseudo(param);
        func.insts.emplace_back(assem::Mov(param_registers[p_counter], asm_param));
        if (++p_counter >= 6) break;
    }

    // get remaining parameters from stack
    for (size_t i=6; i<fp->params.size(); i++) {
        const assem::Stack stk(16 + (i-6)*8);
        const assem::Operand asm_param = assem::Pseudo(fp->params[i]);
        func.insts.emplace_back(assem::Mov(stk, asm_param));
    }

    // insert allocate stack of unknown size first
    func.insts.emplace_back(assem::Allocate_stack(0));

    // translate body of the function
    translate_instructions(func.insts, fp->insts);

    return func;
}
