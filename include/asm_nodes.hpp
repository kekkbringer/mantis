//
// Created by dominik on 7/11/25.
//

#ifndef ASM_NODES_HPP
#define ASM_NODES_HPP

#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "tac.hpp"

/**
 * Holds all classes that make up the assembler program tree.
 */
namespace assem {
    /**
     * Type of register.
     */
    enum class Reg {
        AX, ///< accumulator
        BX, ///< base
        CX, ///< counter
        SP, ///< stack pointer
        BP, ///< stack base pointer
        DI, ///< destination index
        SI, ///< source index
        DX, ///< data

        /** general purpose registers */
        R8, R9, R10, R11, R12, R13, R14, R15
    };

    /**
     * Different condition codes for comparisons.
     */
    enum class Cond_code { E, NE, G, GE, L, LE };

    /**
     * Immediate value like an integer constant
     */
    class Immediate_value {
    public:
        int val; ///< integer value of the immediate value
    };

    /**
     * Register, so far size agnostic.
     */
    class Register {
    public:
        Reg reg; ///< type of register
    };

    /**
     * Pseudo register, only used internally,
     * needs to be replaced with a true register or a stack address at some point or a label to a static variable.
     */
    class Pseudo {
    public:
        std::string_view name; ///< Name of the pseudo register
    };

    /**
     * A stack address.
     */
    class Stack {
    public:
        int offset; ///< offset from %rsp
    };

    /**
     * Represents variables with static storage duration, will later be replaced with e.g. a reference to %rip.
     */
    class Data {
    public:
        std::string_view name;
    };

    /**
     * All possible kinds of operands in the assembler tree.
     */
    using Operand = std::variant<Immediate_value, Register, Pseudo, Stack, Data>;

    /**
     * Types of binary assembler operations.
     */
    enum class Bin_op {
        Add, Sub, Mult, And, Or, Xor, Sal, Sar,
    };

    /**
     * Types of unary assembler operations.
     */
    enum class Un_op {
        Neg, Not,
    };

    /**
     * Mov instruction.
     * Can operate on registers and stack addresses. Source and destination cannot both be stack addresses.
     */
    class Mov {
    public:
        Operand src, dst; ///< source/destination of the mov instruction
        Mov(Operand src, Operand dst) : src(std::move(src)), dst(std::move(dst)) {};
    };

    /**
     * General unary assembler instruction.
     * Can only operate on a register or a stack address.
     */
    class Unary {
    public:
        Un_op op;        ///< type of unary operation
        Operand operand; ///< operand that the operation acts on
        Unary() = default;
        Unary(const Un_op op, Operand dst) : op(op), operand(std::move(dst)) {};
    };

    /**
     * General binary assembler instruction.
     */
    class Binary {
    public:
        Bin_op op;        ///< type of binary operation
        Operand src, dst; ///< left-hand and right-hand side of the binary operation

        Binary(const Bin_op op, Operand s, Operand d) : op(op), src(std::move(s)), dst(std::move(d)) {};
    };

    /**
     * Cmp assembler instruction.
     */
    class Cmp {
    public:
        Operand lhs, rhs; ///< left-hand and right-hand side of the operation

        Cmp(Operand lhs, Operand rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {};
    };

    /**
     * Idiv assembler instruction.
     */
    class Idiv {
    public:
        Operand operand; ///< operand of idiv
    };

    /**
     * Cdq assembly instruction.
     */
    class Cdq {};

    /**
     * Jmp assembly instruction.
     * Unconditional jump to label 'name'.
     */
    class Jmp {
    public:
        std::string_view name; ///< name of the label to jump to.
    };

    /**
     * JmpCC assembly instruction.
     * Conditional jump to label 'name', the condition code 'cc' dictates when to take the jump.
     */
    class JmpCC {
    public:
        Cond_code cc;     ///< condition code to dictate when to take the jump
        std::string_view name; ///< name of the label to jump to.

        JmpCC(const Cond_code cc, const std::string_view name) : cc(cc), name(name) {};
    };

    /**
     * SetCC assembly instruction.
     */
    class SetCC {
    public:
        Cond_code cc;    ///< condition code
        Operand operand; ///< operand of setCC
    };

    /**
     * Assembly label.
     * Represent an assembly label for jump instructions to reference.
     */
    class Label {
    public:
        std::string_view name; ///< name of the label
    };

    /**
     * Allocate stack instruction, that shifts %rsp at the beginning of a function body.
     * This instruction is inserted at the start of every function body and shifts %rsp in such a way, that it points to
     * the top of the stack after all local of the function have been put on the stack.
     */
    class Allocate_stack {
    public:
        int size; ///< size of the function stack
    };

    /**
     * Deallocates stack after at the end of a function.
     */
    class Deallocate_stack { public: int size; };

    /**
     * Push assembly instruction.
     * Pushes the operand onto the stack.
     */
    class Push {
    public:
        Operand operand; ///< Operand to push onto the stack
    };

    /**
     * Call assembly instruction.
     * Instruction to call the function 'name'.
     */
    class Call {
    public:
        std::string_view name; ///< Name of the function to call
    };

    /**
     * Ret assembly instruction
     * The return value of the function must be on EAX. Before the 'ret' instruction, the function epilog is emitted
     * which restores %rsp and %rbp before returning control to the caller.
     */
    class Ret {};

    /**
     * Variant that holds all different instructions of the assembly tree structure.
     */
    using Instruction = std::variant<Mov, Unary, Binary, Cmp, Idiv, Cdq, Jmp, JmpCC, SetCC,
                                     Label, Allocate_stack, Deallocate_stack, Push, Call, Ret>;

    /**
     * Assembly function definition.
     */
    class Function {
    public:
        std::string_view name;               ///< name of the function
        bool global;                    ///< indicates whether the function has global scoping
        std::vector<Instruction> insts; ///< list of assembly instructions that represent the function body
    };

    /**
     * A variable with static storage duration.
     */
    class Static_variable {
    public:
        std::string_view name; ///< name of the static variable
        bool global;      ///< indicates whether the static variable has global scoping
        int init;         ///< initial value of the static variable
    };

    /**
     * Variant that holds all valid top level definition for the assembly program tree structure, namely function
     * definitions and static variables.
     */
    using Top_level = std::variant<Function, Static_variable>;

    /**
     * Assembly program as a list of top level definitions.
     */
    using Program = std::vector<Top_level>;
}

inline std::string to_string(const assem::Cond_code& cc) {
    switch (cc) {
        case assem::Cond_code::E:  return "e";
        case assem::Cond_code::NE: return "ne";
        case assem::Cond_code::L:  return "l";
        case assem::Cond_code::LE: return "le";
        case assem::Cond_code::G:  return "g";
        case assem::Cond_code::GE: return "ge";
    }
    return "UNKNOWN";
}

namespace reg {
    constexpr inline assem::Register AX(assem::Reg::AX);
    constexpr inline assem::Register BX(assem::Reg::BX);
    constexpr inline assem::Register CX(assem::Reg::CX);
    constexpr inline assem::Register SP(assem::Reg::SP);
    constexpr inline assem::Register BP(assem::Reg::BP);
    constexpr inline assem::Register DI(assem::Reg::DI);
    constexpr inline assem::Register SI(assem::Reg::SI);
    constexpr inline assem::Register DX(assem::Reg::DX);

    constexpr inline assem::Register R8(assem::Reg::R8);
    constexpr inline assem::Register R9(assem::Reg::R9);
    constexpr inline assem::Register R10(assem::Reg::R10);
    constexpr inline assem::Register R11(assem::Reg::R11);
    constexpr inline assem::Register R12(assem::Reg::R12);
    constexpr inline assem::Register R13(assem::Reg::R13);
    constexpr inline assem::Register R14(assem::Reg::R14);
    constexpr inline assem::Register R15(assem::Reg::R15);
}

#endif //ASM_NODES_HPP
