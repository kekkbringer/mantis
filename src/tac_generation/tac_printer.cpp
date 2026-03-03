//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"
#include "util.hpp"

#include <iostream>
#include <cassert>

void print_value(const tac::Value& vp, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    if (vp.kind == tac::Value::Kind::Int_constant) {
        std::cout << pre << "INT CONSTANT: " << vp.int_val << "\n";

    } else if (vp.kind == tac::Value::Kind::Variable) {
        std::cout << pre << "VARIABLE: " << vp.name << "\n";

    } else {
        std::cout << "UNKNOWN VALUE\n";
    }

    shift--;
}

void print_instruction(const tac::Inst* ip, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    switch (ip->kind) {
        case tac::Inst::Kind::Return: {
            const auto* ret = cast<tac::Return const>(ip);
            std::cout << pre << "RETURN\n";
            print_value(ret->val, shift);
            break;
        }

        case tac::Inst::Kind::Unary: {
            const auto* un = cast<tac::Unary const>(ip);
            std::cout << pre << "UNARY: " << tac::to_string(un->op) << "\n";
            std::cout << pre << "-from:\n";
            print_value(un->src, shift);
            std::cout << pre << "-to:\n";
            print_value(un->dst, shift);
            break;
        }

        case tac::Inst::Kind::Binary: {
            const auto* bin = cast<tac::Binary const>(ip);
            std::cout << pre << "BINARY: " << tac::to_string(bin->op) << "\n";
            std::cout << pre << "-lhs\n";
            print_value(bin->lhs, shift);
            std::cout << pre << "-rhs\n";
            print_value(bin->rhs, shift);
            std::cout << pre << "-to\n";
            print_value(bin->dst, shift);
            break;
        }

        case tac::Inst::Kind::Copy: {
            const auto* cpy = cast<tac::Copy const>(ip);
            std::cout << pre << "Copy\n";
            std::cout << pre << "-from\n";
            print_value(cpy->src, shift);
            std::cout << pre << "-to\n";
            print_value(cpy->dst, shift);
            break;
        }

        case tac::Inst::Kind::Jump: {
            const auto* jmp = cast<tac::Jump const>(ip);
            std::cout << pre << "JUMP to " << jmp->target << "\n";
            break;
        }

        case tac::Inst::Kind::Jump_if_zero: {
            const auto* jiz = cast<tac::Jump_if_zero const>(ip);
            std::cout << pre << "JUMP_IF_ZERO to " << jiz->target << "\n";
            print_value(jiz->cond, shift);
            break;
        }

        case tac::Inst::Kind::Jump_if_not_zero: {
            const auto* jinz = cast<tac::Jump_if_not_zero const>(ip);
            std::cout << pre << "JUMP_IF_NOT_ZERO to " << jinz->target << "\n";
            print_value(jinz->cond, shift);
            break;
        }

        case tac::Inst::Kind::Label: {
            const auto* lab = cast<tac::Label const>(ip);
            std::cout << pre << "LABEL: " << lab->name << "\n";
            break;
        }

        case tac::Inst::Kind::Function_call: {
            const auto* fun_call = cast<tac::Function_call const>(ip);
            std::cout << pre << "FUNCTION CALL with name " << fun_call->name << "\n";
            std::cout << pre << "-parameters:\n" ;
            for (const auto& a: fun_call->arguments()) print_value(a, shift);
            std::cout << pre << "-to\n" ;
            print_value(fun_call->dst, shift);
            break;
        }
    }

    shift--;
}

void print_function(const tac::Function* fp, int& shift) {
    if (fp == nullptr) return;

    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    std::cout << pre << "FUNCTION: " << fp->name << "\n";
    std::cout << pre << "-is global: " << (fp->global ? "yes" : "no") << "\n";
    std::cout << pre << "-parameter: ";
    for (const auto& p: fp->parameters()) std::cout << p << ", ";
    std::cout << "\n";
    std::cout << pre << "-body:\n";
    for (const auto& ip: fp->instructions()) {
        print_instruction(ip, shift);
    }

    shift--;
}

void print_static_variable(const tac::Static_variable* svp, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift; i++) pre += "   ";

    std::cout << pre << "STATIC VARIABLE: " << svp->name << "\n";
    std::cout << pre << "-is global: " << (svp->global ? "yes" : "no") << "\n";
    std::cout << pre << "-init: " << svp->init << "\n";

    shift--;
}

void print_top_level(const tac::Top_level& tl) {
    int shift = 0;
    
    if (tl.kind == tac::Top_level::Kind::Function) {
        print_function(tl.func, shift);

    } else if (tl.kind == tac::Top_level::Kind::Static_variable) {
        print_static_variable(tl.stat_var, shift);
    }
}

void Tac_generator::print_tac(const tac::Program* prog) {
    std::cout << ":: TAC PROGRAM HEAD\n";

    for (const auto& tl: prog->top_level()) {
        print_top_level(tl);
    }

}
