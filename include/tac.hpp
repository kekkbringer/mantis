//
// Created by dominik on 3/1/26.
//

#ifndef TAC_HPP
#define TAC_HPP

#include <string_view>
#include <span>
#include <cstddef>

#include "tac_unary_operator.hpp"
#include "tac_binary_operator.hpp"

/**
 * Holds all structs that make up the three address code (TAC) program tree structure.
 */
namespace tac {
    /**
     * Any TAC value like integer constant and variables.
     */
    struct Value {
        enum class Kind {
            Int_constant, // integer constant
            Variable,     // variable
        } kind; ///< indicates kind of value

        union {
            int int_val;           ///< value of an integer constant
            std::string_view name; ///< name of a variable
        };

    private:
        Value() : kind(Kind::Int_constant), int_val(0) {} // used in factory methods

    public:
        /**
         * Factory method for a TAC integer constant.
         */
        static constexpr Value make_int(const int val) {
            Value v; v.kind = Kind::Int_constant; v.int_val = val; return v;
        }

        /**
         * Factory method for a TAC variable.
         */
        static constexpr Value make_variable(const std::string_view name) {
            Value v; v.kind = Kind::Variable; v.name = name; return v;
        }
    };

    /**
     * Base class for all TAC instructions.
     */
    struct Inst {
        enum class Kind {
            Return,
            Unary,
            Binary,
            Copy,
            Jump,
            Jump_if_zero,
            Jump_if_not_zero,
            Label,
            Function_call,
        } kind; ///< indicates kind of TAC instruction

    protected:
        explicit Inst(const Kind k) : kind(k) {}
    };

    /**
     * TAC return instruction.
     */
    struct Return : Inst {
        Value val; ///< value to return

        explicit Return(const Value &val)
            : Inst(Kind::Return), val(val) {}
    };

    /**
     * TAC unary operation instruction.
     */
    struct Unary : Inst {
        Unary_operator op; ///< type of unary operation
        Value src;         ///< source operand
        Value dst;         ///< destination operand

        Unary(const Unary_operator op, const Value &src, const Value &dst)
            : Inst(Kind::Unary), op(op), src(src), dst(dst) {}
    };

    /**
     * TAC binary operation instruction.
     */
    struct Binary : Inst {
        Binary_operator op; ///< type of binary operation
        Value lhs;          ///< left-hand side operand
        Value rhs;          ///< right-hand side operand
        Value dst;          ///< destination operand

        Binary(const Binary_operator op, const Value &lhs, const Value &rhs, const Value &dst)
            : Inst(Kind::Binary), op(op), lhs(lhs), rhs(rhs), dst(dst) {}
    };

    /**
     * TAC copy instruction.
     */
    struct Copy : Inst {
        Value src; ///< value to copy
        Value dst; ///< destination to copy to

        Copy(const Value &src, const Value &dst)
            : Inst(Kind::Copy), src(src), dst(dst) {}
    };

    /**
     * TAC unconditional jump instruction.
     */
    struct Jump : Inst {
        std::string_view target; ///< interned name of the target label

        explicit Jump(const std::string_view target)
            : Inst(Kind::Jump), target(target) {}
    };

    /**
     * TAC jump if zero instruction.
     */
    struct Jump_if_zero : Inst {
        Value cond;              ///< condition value, jump taken if zero
        std::string_view target; ///< interned name of the target label

        Jump_if_zero(const Value &cond, const std::string_view target)
            : Inst(Kind::Jump_if_zero), cond(cond), target(target) {}
    };

    /**
     * TAC jump if not zero instruction.
     */
    struct Jump_if_not_zero : Inst {
        Value cond;              ///< condition value, jump taken if not zero
        std::string_view target; ///< interned name of the target label

        Jump_if_not_zero(const Value &cond, const std::string_view target)
            : Inst(Kind::Jump_if_not_zero), cond(cond), target(target) {}
    };

    /**
     * TAC label instruction, acts as target for jump instructions.
     */
    struct Label : Inst {
        std::string_view name; ///< interned name of the label

        explicit Label(const std::string_view name)
            : Inst(Kind::Label), name(name) {}
    };

    /**
     * TAC function call instruction.
     */
    struct Function_call : Inst {
        std::string_view name; ///< interned name of the function to call
        Value* args;           ///< arena-allocated array of argument values
        size_t n_args;         ///< number of arguments
        Value dst;             ///< destination for the return value

        Function_call(const std::string_view name, Value* args, const size_t n_args, const Value &dst)
            : Inst(Kind::Function_call), name(name), args(args), n_args(n_args), dst(dst) {}

        /**
         * Constructs and returns a span over all arguments of the function call for e.g. a range based for loop.
         * @return std::spawn over arguments
         */
        [[nodiscard]] std::span<Value> arguments() const { return {args, n_args}; }
    };

    /**
     * A TAC function node.
     */
    struct Function {
        std::string_view name;    ///< name of the function
        bool global;              ///< indicates whether the function is global
        std::string_view* params; ///< array of names of parameters
        size_t n_params;          ///< number of parameters
        Inst** insts;             ///< arena allocated array of pointer to the instructions in the function body
        size_t n_insts;           ///< number of instructions in function

        Function(const std::string_view n, const bool g, std::string_view* p, const size_t np, Inst** i, const size_t ni)
            : name(n), global(g), params(p), n_params(np), insts(i), n_insts(ni) {}

        [[nodiscard]] std::span<std::string_view> parameters() const { return {params, n_params}; }
        [[nodiscard]] std::span<Inst*> instructions() const { return {insts, n_insts}; }
    };

    /**
     * TAC node for a variable with static storage duration.
     */
    struct Static_variable {
        std::string_view name; ///< name of the static variable
        bool global;           ///< indicates whether the variable is global
        int init;              ///< initial value of the variable

        Static_variable(const std::string_view n, const bool g, const int i) : name(n), global(g), init(i) {}
    };

    /**
     * A TAC top level construct, either a function or a variable with static storage duration.
     */
    struct Top_level {
        enum class Kind {Function, Static_variable} kind; ///< kind of top level construct
        union {
            Function* func;            ///< pointer to actual item
            Static_variable* stat_var; ///< pointer to actual item
        };
        explicit Top_level(Function* f) : kind(Kind::Function), func(f) {}
        explicit Top_level(Static_variable* sv) : kind(Kind::Static_variable), stat_var(sv) {}
    };

    /**
     * The root node of the TAC tree, representing the whole translation unit.
     */
    struct Program {
        Top_level* top_level_constructs; ///< arena allocated array of top level constructs
        size_t n_top_level_constructs;   ///< number of top level constructs
        Program(Top_level* tl, const size_t ntl) : top_level_constructs(tl), n_top_level_constructs(ntl) {}

        /**
         * Constructs and returns a span over all top-level constructs for e.g. a range based for loop.
         * @return std::spawn over top level constructs
         */
        [[nodiscard]] std::span<Top_level const> top_level() const { return std::span(top_level_constructs, n_top_level_constructs); }
    };
}

#endif //TAC_HPP
