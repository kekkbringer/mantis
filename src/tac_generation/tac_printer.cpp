//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"

#include <iostream>
#include <cassert>

void print_value(const tac::Value_ptr& vp, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, tac::Constant>) {
            std::cout << pre << "Constant: " << arg.val << "\n";
        } else if constexpr (std::is_same_v<T, tac::Variable>) {
            std::cout << pre << "Variable: " << arg.name << "\n";
        } else {
            assert("false" && "internal error, impossible branch");
        }
    }, *vp);

    shift--;
}

void print_instruction(const tac::Instruction_ptr& ip, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, tac::Return_ptr>) {
            std::cout << pre << "Return\n";
            print_value(arg->val, shift);

        // unary
        } else if constexpr (std::is_same_v<T, tac::Unary_ptr>) {
            std::cout << pre << "Unary: " << tac::to_string(arg->op) << "\n";
            print_value(arg->src, shift);
            std::cout << pre << "   to\n";
            print_value(arg->dst, shift);

        // binary
        } else if constexpr (std::is_same_v<T, tac::Binary_ptr>) {
            std::cout << pre << "Binary: " << tac::to_string(arg->op) << "\n";
            print_value(arg->lhs, shift);
            std::cout << pre << "   and\n";
            print_value(arg->rhs, shift);
            std::cout << pre << "   to\n";
            print_value(arg->dst, shift);

        // label
        } else if constexpr (std::is_same_v<T, tac::Label_ptr>) {
            std::cout << pre << "Label: " << arg->name << "\n";

        // jump
        } else if constexpr (std::is_same_v<T, tac::Jump_ptr>) {
            std::cout << pre << "Jump to " << arg->target << "\n";

        // jump if zero
        } else if constexpr (std::is_same_v<T, tac::Jump_if_zero_ptr>) {
            std::cout << pre << "Jump_if_zero to " << arg->target << "\n";
            print_value(arg->cond, shift);

        // jump if not zero
        } else if constexpr (std::is_same_v<T, tac::Jump_if_not_zero_ptr>) {
            std::cout << pre << "Jump_if_not_zero to " << arg->target << "\n";
            print_value(arg->cond, shift);

        // copy
        } else if constexpr (std::is_same_v<T, tac::Copy_ptr>) {
            std::cout << pre << "Copy\n";
            print_value(arg->src, shift);
            std::cout << pre << "   to\n";
            print_value(arg->dst, shift);

        // function call
        } else if constexpr (std::is_same_v<T, tac::Function_call_ptr>) {
            std::cout << pre << "Function call with name " << arg->name << "\n";
            for (const auto& a: arg->args) print_value(a, shift);
            std::cout << pre << "   to\n" ;
            print_value(arg->dst, shift);

        } else {
            assert(false && "print_instruction not fully implemented yet");
        }
    }, ip);

    shift--;
}

void print_function(const tac::Function_ptr& fp, int& shift) {
    if (fp == nullptr) return;

    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    std::cout << pre << "Function: " << fp->name << "\n";
    for (const auto& ip: fp->insts) {
        print_instruction(ip, shift);
    }

    shift--;
}

void print_static_variable(const tac::Static_variable_ptr& svp, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    std::cout << pre << "Static variable: " << svp->name << "\n";
    std::cout << pre << "   is global: " << (svp->global ? "yes" : "no") << "\n";
    std::cout << pre << "   init: " << svp->init << "\n";

    shift--;
}

void print_top_level(const tac::Top_level_ptr& tlp) {
    int shift = 0;
    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, tac::Function_ptr>) {
            print_function(arg, shift);
        } else if constexpr (std::is_same_v<T, tac::Static_variable_ptr>) {
            print_static_variable(arg, shift);
        } else {
            assert(false && "internal error, impossible branch");
        }
    }, tlp);
}

void Tac_generator::print_tac(const tac::Program &prog) {
    std::cout << ":: TAC PROGRAM HEAD\n";

    for (const auto& tl: prog) {
        print_top_level(tl);
    }

}
