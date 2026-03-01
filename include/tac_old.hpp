//
// Created by dominik on 7/8/25.
//

#ifndef TAC_OLD_HPP
#define TAC_OLD_HPP

#include <utility>
#include <variant>
#include <memory>
#include <vector>

#include "tac_unary_operator.hpp"
#include "tac_binary_operator.hpp"

/**
 * Holds all classes that make up the three address code (TAC) program tree structure.
 */
namespace tac_old {
    class Function;          typedef std::unique_ptr<Function> Function_ptr;
    class Static_variable;   typedef std::unique_ptr<Static_variable> Static_variable_ptr;
    using Top_level_ptr = std::variant<Function_ptr, Static_variable_ptr>;

    using Program = std::vector<Top_level_ptr>;

    // instructions
    class Return;            typedef std::unique_ptr<Return> Return_ptr;
    class Unary;             typedef std::unique_ptr<Unary> Unary_ptr;
    class Binary;            typedef std::unique_ptr<Binary> Binary_ptr;
    class Copy;              typedef std::unique_ptr<Copy> Copy_ptr;
    class Jump;              typedef std::unique_ptr<Jump> Jump_ptr;
    class Jump_if_zero;      typedef std::unique_ptr<Jump_if_zero> Jump_if_zero_ptr;
    class Jump_if_not_zero;  typedef std::unique_ptr<Jump_if_not_zero> Jump_if_not_zero_ptr;
    class Label;             typedef std::unique_ptr<Label> Label_ptr;
    class Function_call;     typedef std::unique_ptr<Function_call> Function_call_ptr;
    using Instruction_ptr = std::variant<Return_ptr, Unary_ptr, Binary_ptr, Copy_ptr, Jump_ptr,
                                         Jump_if_zero_ptr, Jump_if_not_zero_ptr, Label_ptr, Function_call_ptr>;

    // values
    class Constant; typedef std::unique_ptr<Constant> Constant_ptr;
    class Variable; typedef std::unique_ptr<Variable> Variable_ptr;

    /**
     * TAC constant node.
     * So far only integer constants are considered.
     */
    class Constant {
    public:
        int val; ///< value of the integer constant

        Constant() : val(0) {}
        explicit Constant(const int val) : val(val) {}
    };

    /**
     * TAC variable node.
     */
    class Variable {
    public:
        std::string name; ///< name of the variable

        Variable() : name("") {}
        explicit Variable(const std::string& name) : name(name) {};
    };

    /**
     * Variant that holds the different kinds of value nodes of the TAC tree. A TAC value can either be a constant or
     * a variable.
     */
    using Value = std::variant<std::monostate, Constant, Variable>;

    /**
     * Smart pointer to a TAC value.
     */
    using Value_ptr = std::unique_ptr<Value>;

    /**
     * TAC function node.
     * This is one of the possible top level constructs of a TAC program.
     */
    class Function {
    public:
        std::string name; ///< name of the function
        bool global = false;
        std::vector<std::string> params;
        std::vector<Instruction_ptr> insts; ///< list of pointer to the instructions that make up the function body
    };

    /**
     * TAC static variable node.
     * This is one of the possible top level constructs for a TAC program.
     */
    class Static_variable {
    public:
        std::string name; ///< name of the static variable
        bool global; ///< indicates whether the static variable has global scoping
        int init; ///< initial value of the static variable
    };

    /**
     * TAC return statement node.
     */
    class Return {
    public:
        Value_ptr val; ///< value to be returned

        explicit Return(const Value& val) : val(std::make_unique<Value>(val)) {}
    };

    /**
     * General TAC unary operation node.
     */
    class Unary {
    public:
        Unary_operator op; ///< type of the unary operation
        Value_ptr src; ///< source that the unary operation acts on
        Value_ptr dst; ///< destination, where the result of the unary expression will be stored.

        Unary(const Unary_operator op, const Value& src, const Value& dst)
        : op(op), src(std::make_unique<Value>(src)), dst(std::make_unique<Value>(dst)) {}
    };

    /**
     * General TAC binary expression node.
     */
    class Binary {
    public:
        Binary_operator op; ///< type of the binary operation
        Value_ptr lhs; ///< left-hand side of the binary operation
        Value_ptr rhs; ///< right-hand side of the binary operation
        Value_ptr dst; ///< destination of the result of the binary expression

        Binary(const Binary_operator op, const Value& lhs, const Value& rhs, const Value& dst)
        : op(op), lhs(std::make_unique<Value>(lhs)), rhs(std::make_unique<Value>(rhs)), dst(std::make_unique<Value>(dst)) {}
    };

    /**
     * TAC copy instruction node.
     */
    class Copy {
    public:
        Value_ptr src; ///< value to be copied to the destination
        Value_ptr dst; ///< target to of copy instruction

        Copy(const Value& src, const Value& dst) : src(std::make_unique<Value>(src)), dst(std::make_unique<Value>(dst)) {}
    };

    /**
     * TAC jump statement node.
     */
    class Jump {
    public:
        std::string target; ///< name of the target label to be jumped to
    };

    /**
     * TAC jump if zero statement node.
     */
    class Jump_if_zero {
    public:
        Value_ptr cond; ///< if condition is zero, jump will be taken
        std::string target; ///< name of the target label to be jumped to

        Jump_if_zero(const Value& c, std::string t) : cond(std::make_unique<Value>(c)), target(std::move(t)) {}
    };

    /**
     * TAC jump if not zero statement node.
     */
    class Jump_if_not_zero {
    public:
        Value_ptr cond; ///< if condition is not zero, jump will be taken
        std::string target; ///< name of the target label to be jumped to

        Jump_if_not_zero(const Value& c, std::string t) : cond(std::make_unique<Value>(c)), target(std::move(t)) {}
    };

    /**
     * TAC label node.
     * Acts as target for jump statements.
     */
    class Label {
    public:
        std::string name; ///< name of the label
    };

    /**
     * TAC function call node.
     */
    class Function_call {
    public:
        std::string name; ///< name of the function to be called
        std::vector<Value_ptr> args; ///< arguments to be passed to the function
        Value_ptr dst; ///< destination where return value of the function will be save to
    };
}

#endif //TAC_OLD_HPP
