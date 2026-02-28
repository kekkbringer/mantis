//
// Created by dominik on 7/11/25.
//

#include <cassert>

#include "asm_generator.hpp"

#include <iostream>

/**
 * Prints an assem::Register to std::cout.
 * @param reg Register to be printed
 */
void print_register(const assem::Register& reg) {
    std::cout << "      Register ";
    switch (reg.reg) {
        case assem::Reg::AX: std::cout << "AX"; break;
        case assem::Reg::BX: std::cout << "BX"; break;
        case assem::Reg::CX: std::cout << "CX"; break;
        case assem::Reg::DX: std::cout << "DX"; break;
        case assem::Reg::SP: std::cout << "SP"; break;
        case assem::Reg::BP: std::cout << "BP"; break;
        case assem::Reg::DI: std::cout << "DI"; break;
        case assem::Reg::SI: std::cout << "SI"; break;
        case assem::Reg::R8: std::cout << "R8"; break;
        case assem::Reg::R9: std::cout << "R9"; break;
        case assem::Reg::R10: std::cout << "R10"; break;
        case assem::Reg::R11: std::cout << "R11"; break;
        case assem::Reg::R12: std::cout << "R12"; break;
        case assem::Reg::R13: std::cout << "R13"; break;
        case assem::Reg::R14: std::cout << "R14"; break;
        case assem::Reg::R15: std::cout << "R15"; break;
    }
    std::cout << "\n";
}

/**
 * Prints an assem::Operand to std::cout.
 * @param op operand to be printed
 */
void print_operand(const assem::Operand& op) {
    std::visit([]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, assem::Immediate_value>) {
            std::cout << "      Immediate_value " << arg.val << "\n";
        } else if constexpr (std::is_same_v<T, assem::Register>) {
            print_register(arg);
        } else if constexpr (std::is_same_v<T, assem::Pseudo>) {
            std::cout << "      Pseudo " << arg.name << "\n";
        } else if constexpr (std::is_same_v<T, assem::Stack>) {
            std::cout << "      Stack " << arg.offset << "\n";
        } else if constexpr (std::is_same_v<T, assem::Data>) {
            std::cout << "      Data " << arg.name << "\n";
        }
    }, op);
}

/**
 * Prints an assem::Instruction to std::cout.
 * @param i instruction to be printed
 */
void print_instruction(const assem::Instruction& i) {
    std::visit([]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, assem::Mov>) {
            std::cout << "   Mov\n";
            print_operand(arg.src);
            print_operand(arg.dst);
        } else if constexpr (std::is_same_v<T, assem::Ret>) {
            std::cout << "   Ret\n";

        // unary
        } else if constexpr (std::is_same_v<T, assem::Unary>) {
            std::cout << "   Unary ";
            switch (arg.op) {
                case assem::Un_op::Neg: std::cout << "Neg\n"; break;
                case assem::Un_op::Not: std::cout << "Not\n"; break;
                default: assert(false && "unop error");
            }
            print_operand(arg.operand);

        // binary
        } else if constexpr (std::is_same_v<T, assem::Binary>) {
            std::cout << "   Binary ";
            switch (arg.op) {
                case assem::Bin_op::Add: std::cout << "Add\n"; break;
                case assem::Bin_op::Sub: std::cout << "Sub\n"; break;
                case assem::Bin_op::Mult: std::cout << "Mult\n"; break;
                case assem::Bin_op::And: std::cout << "And\n"; break;
                case assem::Bin_op::Or: std::cout << "Or\n"; break;
                case assem::Bin_op::Xor: std::cout << "Xor\n"; break;
                case assem::Bin_op::Sal: std::cout << "Sal\n"; break;
                case assem::Bin_op::Sar: std::cout << "Sar\n"; break;
                default: assert(false && "binop error");
            }
            print_operand(arg.src);
            print_operand(arg.dst);

        // division
        } else if constexpr (std::is_same_v<T, assem::Idiv>) {
            std::cout << "   Idiv\n";
            print_operand(arg.operand);

        // sign extension
        } else if constexpr (std::is_same_v<T, assem::Cdq>) {
            std::cout << "   Cdq\n";

        // label
        } else if constexpr (std::is_same_v<T, assem::Label>) {
            std::cout << "   Label " << arg.name << "\n";

        // jmp
        } else if constexpr (std::is_same_v<T, assem::Jmp>) {
            std::cout << "   Jmp " << " to " << arg.name << "\n";

        // jmp cc
        } else if constexpr (std::is_same_v<T, assem::JmpCC>) {
            std::cout << "   JmpCC " << to_string(arg.cc) << " to " << arg.name << "\n";

        // set cc
        } else if constexpr (std::is_same_v<T, assem::SetCC>) {
            std::cout << "   SetCC " << to_string(arg.cc) << "\n";
            print_operand(arg.operand);

        // cmp
        } else if constexpr (std::is_same_v<T, assem::Cmp>) {
            std::cout << "   Cmp\n";
            print_operand(arg.lhs);
            print_operand(arg.rhs);

        // sign extension
        } else if constexpr (std::is_same_v<T, assem::Cdq>) {
            std::cout << "   Cdq\n";

        // allocate stack
        } else if constexpr (std::is_same_v<T, assem::Allocate_stack>) {
            std::cout << "   Allocate_stack of size " << arg.size << "\n";

        // deallocate stack
        } else if constexpr (std::is_same_v<T, assem::Deallocate_stack>) {
            std::cout << "   Deallocate_stack of size " << arg.size << "\n";

        // function call
        } else if constexpr (std::is_same_v<T, assem::Call>) {
            std::cout << "   Call " << arg.name << "\n";

        // push
        } else if constexpr (std::is_same_v<T, assem::Push>) {
            std::cout << "   Push\n";
            print_operand(arg.operand);

        } else {
            assert(false && "error in print_instruction");
        }
    }, i);
}

/**
 * Prints an assem::Top_level to std::cout.
 * @param tl top level construct to be printed
 */
void print_top_level(const assem::Top_level& tl) {
    std::visit([&]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, assem::Function>) {
            std::cout << "Function: " << arg.name << "\n";
            std::cout << "  global: " << arg.global << "\n";
            for (const auto& i: arg.insts) print_instruction(i);
        } else if constexpr (std::is_same_v<T, assem::Static_variable>) {
            std::cout << "Static_variable\n";
        }
    }
    ,tl);
}

/**
 * Prints an assembly program tree to standard output by traversing the tree and printing each node.
 * @param prog assembly program to be printed
 */
void Asm_generator::print_asm(const assem::Program& prog) {
    std::cout << " :: ASM TREE\n";

    for (const auto& tl: prog) {
        print_top_level(tl);
    }
}
