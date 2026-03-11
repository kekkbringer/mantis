//
// Created by dominik on 7/12/25.
//

#include "code_emitter.hpp"

#include <cassert>

/**
 * Translated an assembly register to a string depending on the type and size of register.
 * @param reg assembly register to be translated to a string
 * @param size size of the operand in byte, important for register naming
 * @return name of register as string
 */
std::string to_string(const assem::Register& reg, const int size) {
    using namespace assem;
    if (size == 1)
    switch (reg.reg) {
        case Reg::AX: return "al"; case Reg::BX: return "bl";
        case Reg::CX: return "cl"; case Reg::DX: return "dl";
        case Reg::SP: return "spl"; case Reg::BP: return "bpl";
        case Reg::DI: return "dil"; case Reg::SI: return "sil";
        case Reg::R8: return "r8b"; case Reg::R9: return "r9b";
        case Reg::R10: return "r10b"; case Reg::R11: return "r11b";
        case Reg::R12: return "r12b"; case Reg::R13: return "r13b";
        case Reg::R14: return "r14b"; case Reg::R15: return "r15b";
    }
    else if (size == 4)
    switch (reg.reg) {
        case Reg::AX: return "eax"; case Reg::BX: return "ebx";
        case Reg::CX: return "ecx"; case Reg::DX: return "edx";
        case Reg::SP: return "esp"; case Reg::BP: return "ebp";
        case Reg::DI: return "edi"; case Reg::SI: return "esi";
        case Reg::R8: return "r8d"; case Reg::R9: return "r9d";
        case Reg::R10: return "r10d"; case Reg::R11: return "r11d";
        case Reg::R12: return "r12d"; case Reg::R13: return "r13d";
        case Reg::R14: return "r14d"; case Reg::R15: return "r15d";
    }
    else if (size == 8)
        switch (reg.reg) {
        case Reg::AX: return "rax"; case Reg::BX: return "rbx";
        case Reg::CX: return "rcx"; case Reg::DX: return "rdx";
        case Reg::SP: return "rsp"; case Reg::BP: return "rbp";
        case Reg::DI: return "rdi"; case Reg::SI: return "rsi";
        case Reg::R8: return "r8"; case Reg::R9: return "r9";
        case Reg::R10: return "r10"; case Reg::R11: return "r11";
        case Reg::R12: return "r12"; case Reg::R13: return "r13";
        case Reg::R14: return "r14"; case Reg::R15: return "r15";
    }

    assert(false && "internal error in to_string register");
    return "INTERNAL ERROR IN CODE EMISSION OF OPERAND";
}

/**
 * Emits an assembly operand, valid possibilities are immediate values, register and stack addresses. All pseudo
 * register need to be replaced at this point since they do not represent legal assembly operands.
 * @param o assembly operand to be emitted
 * @param size size of the operand in byte, important for register naming
 */
void Code_emitter::emit_operand(const assem::Operand& o, const int size) {
    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, assem::Immediate_value>) {
            os << "$" << arg.val;
        } else if constexpr (std::is_same_v<T, assem::Register>) {
            os << "%" << to_string(arg, size);
        } else if constexpr (std::is_same_v<T, assem::Pseudo>) {
            os << "pseudo " << arg.name;
            //assert(false && "internal error in emit_instruction: pseudo");
        } else if constexpr (std::is_same_v<T, assem::Stack>) {
            os << arg.offset << "(%rbp)";
        } else if constexpr (std::is_same_v<T, assem::Data>) {
            os << arg.name << "(%rip)";
        } else {
            assert(false && "internal error in emit_instruction");
        }
    }, o);
}
