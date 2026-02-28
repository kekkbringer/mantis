//
// Created by dominik on 8/10/25.
//

#include "asm_generator.hpp"

#include <array>

void Asm_generator::translate_function_call(const tac::Function_call_ptr& fun_call, std::vector<assem::Instruction>& ainsts) {
    constexpr std::array arg_registers = {reg::DI, reg::SI, reg::DX, reg::CX, reg::R8, reg::R9};

    const auto& tac_args = fun_call->args;

    // adjust stack alignment
    int stack_padding = 0;
    if (tac_args.size() > 6 and tac_args.size() % 2 == 1) stack_padding = 8;
    if (stack_padding != 0) ainsts.emplace_back(assem::Allocate_stack(stack_padding));

    // pass args in registers
    int reg_index = 0;
    for (const auto& tac_arg: tac_args) {
        if (reg_index >= 6) break;
        const auto asm_arg = translate_value(tac_arg);
        ainsts.emplace_back(assem::Mov(asm_arg, arg_registers[reg_index++]));
    }

    // pass args on stack
    for (int index=tac_args.size()-1; index>5; index--) {
        const auto asm_arg = translate_value(tac_args[index]);
        std::visit([&]<class T0>(T0&& arg) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, assem::Register> or std::is_same_v<T, assem::Immediate_value>)
                ainsts.emplace_back(assem::Push(arg));
            else {
                ainsts.emplace_back(assem::Mov(arg, reg::AX));
                ainsts.emplace_back(assem::Push(reg::AX));
            }
        }, asm_arg);
    }

    // emit call instruction
    ainsts.emplace_back(assem::Call(fun_call->name));

    // adjust stack pointer
    const int n_stack_args = tac_args.size() > 6 ? tac_args.size()-6 : 0;
    int bytes_to_remove = 8 * n_stack_args + stack_padding;
    if (bytes_to_remove != 0) ainsts.emplace_back(assem::Deallocate_stack(bytes_to_remove));

    // retrieve return value
    auto func_dst = translate_value(fun_call->dst);
    ainsts.emplace_back(assem::Mov(reg::AX, func_dst));

}
