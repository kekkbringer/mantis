//
// Created by dominik on 2/25/26.
//

#ifndef UNTITLED_AST_HPP
#define UNTITLED_AST_HPP

#include <cstddef>
#include <span>
#include <string_view>

#include "ast_unary_operator.hpp"
#include "ast_binary_operator.hpp"
#include "source_location.hpp"

/**
 * This namespace encapsulates all nodes from the abstract syntax tree (AST).
 */
namespace ast {
    /**
     * Base class for a declaration node in the AST. It has a kind tag to indicate what kind of declaration it is and
     * always holds a Source_location to the source file. All declarations will be trivially destructible so they can be
     * stored in the arena allocator.
     */
    struct Decl {
        enum class Kind {Var, Func, Error} kind; ///< kind of the declaration
        std::string_view name;                   ///< interned identifier of the declaration
        enum class Storage_class {
            None,
            Static,
            Extern
        } storage_class;                         ///< storage class of the declaration
        Source_location loc;                     ///< location of the declaration in the source file

    protected:
        Decl(const Kind k, const std::string_view n, const Storage_class sc, const Source_location& sl)
            : kind(k), name(n), storage_class(sc), loc(sl) {}
    };

    /**
     * Base class for all expression nodes in the AST. It has a kind tag to indicate what kind of expression it is and
     * always holds a Source_location to the source file. All expressions will be trivially destructible so they can be
     * stored in the arena allocator.
     */
    struct Expr {
        enum class Kind {
            Constant,      ///< number constant, e.g. 3
            Variable,      ///< variable, e.g. x
            Unary,         ///< unary expression, e.g. ~c
            Binary,        ///< binary expression, e.g. x * y
            Assignment,    ///< assignment, e.g. x = 1
            Conditional,   ///< ternary conditional, e.g. c ? y : n
            Function_call, ///< function call, e.g. kek(a, b)
            Inc_dec,       ///< increment/decrement prefix/suffix, e.g. x++
            Error,         ///< error expression
        } kind; ///< enum tag to indicate kind of expression

        Source_location loc; ///< location of the expression in the source file

    protected:
        Expr(const Kind k, const Source_location& sl) : kind(k), loc(sl) {}

    public:
        /**
         * This function returns whether an expression is a valid lvalue.
         */
        [[nodiscard]] inline bool is_lvalue() const { return kind == Kind::Variable; }
    };

    /**
     * Base class for all statement nodes in the AST. It has a kind tag to indicate what kind of statement it is and
     * always holds a Source_location to the source file. All expressions will be trivially destructible so they can be
     * stored in the arena allocator.
     */
    struct Stmt {
        enum class Kind {
            Return,       ///< return statement
            If,           ///< if statement
            Compound,     ///< a compound statement
            Break,        ///< break statement both for loops and switches
            Continue,     ///< continue statement
            While,        ///< while loop statement
            Do_while,     ///< do while loop statement
            For,          ///< for loop statement
            Null,         ///< null statement
            Labeled,      ///< any labeled statement (not including switch labels like case and default)
            Goto,         ///< goto statement
            Switch,       ///< a switch statement
            Switch_label, ///< for both, 'case' and 'default' labels
            Expr,         ///< an expression statement
            Error,        ///< node for error recovery
        } kind; ///< enum tag to indicate kind of statement

        Source_location loc; ///< location of the statement in the source file

    protected:
        Stmt(const Kind k, const Source_location& sl) : kind(k), loc(sl) {}
    };

    /**
     * Integer constant expression.
     */
    struct Constant : Expr {
        int val; ///< value of the constant
        Constant(const int val, const Source_location& sl) : Expr(Kind::Constant, sl), val(val) {}
    };

    /**
     * A variable expression.
     */
    struct Variable : Expr {
        std::string_view name; ///< string_view to the name of the variable
        Variable(const std::string_view name, const Source_location& sl) : Expr(Kind::Variable, sl), name(name) {}
    };

    /**
     * A unary operation expression.
     */
    struct Unary : Expr {
        ast::Unary_operator op; ///< type of unary operation
        Expr* operand;     ///< expression the unary operation acts on
        Unary(const ast::Unary_operator op, Expr* operand, const Source_location& sl)
            : Expr(Kind::Unary, sl), op(op), operand(operand) {}
    };

    /**
     * A binary operation expression.
     */
    struct Binary : Expr {
        ast::Binary_operator op; ///< type of the binary expression
        Expr* lhs;               ///< left-hand side
        Expr* rhs;               ///< right-hand side
        Binary(const ast::Binary_operator op, Expr* lhs, Expr* rhs, const Source_location& sl)
            : Expr(Kind::Binary, sl), op(op), lhs(lhs), rhs(rhs) {}
    };

    /**
     * An assignment expression.
     */
    struct Assignment : Expr {
        Expr* lhs; ///< left-hand side
        Expr* rhs; ///< right-hand side
        Assignment(Expr* lhs, Expr* rhs, const Source_location& sl)
            : Expr(Kind::Assignment, sl), lhs(lhs), rhs(rhs) {}
    };

    /**
     * Ternary conditional expression.
     */
    struct Conditional : Expr {
        Expr* cond;  ///< condition to evaluate
        Expr* then;  ///< 'true' branch
        Expr* else_; ///< 'false' branch
        Conditional(Expr* cond, Expr* then, Expr* else_, const Source_location& sl)
            : Expr(Kind::Conditional, sl), cond(cond), then(then), else_(else_) {}
    };

    /**
     * A function call expression.
     */
    struct Function_call : Expr {
        std::string_view name; ///< name of the function being called
        Expr** args;           ///< array of pointer of argument expression
        size_t n_args;         ///< number of arguments
        Function_call(const std::string_view name, Expr** args, const size_t n_args, const Source_location& sl)
            : Expr(Kind::Function_call, sl), name(name), args(args), n_args(n_args) {}

        /**
         * Constructs and returns a span over all top-level declarations for e.g. range based for loops.
         * @return std::span over all top-level declarations
         */
        [[nodiscard]] std::span<Expr* const> arguments() const { return std::span(args, n_args); }
    };

    /**
     * An increment or decrement prefix or suffix.
     */
    struct Inc_dec : Expr {
        Expr* operand;     ///< expression to increment/decrement
        bool is_increment; ///< true if increment, false if decrement
        bool is_prefix;    ///< true if prefix, false if suffix
        Inc_dec(Expr* op, const bool is_inc, const bool is_pre, const Source_location& sl)
            : Expr(Kind::Inc_dec, sl), operand(op), is_increment(is_inc), is_prefix(is_pre) {}
    };

    /**
     * Error expression, which is returned when parsing an expression results in a catastrophic error after which the
     * error recovery mode should be entered.
     */
    struct Error_expr : Expr {
        explicit Error_expr(const Source_location& sl) : Expr(Kind::Error, sl) {}
    };

    /**
     * Error declaration, which is returned when parsing a declaration results in a catastrophic error after
     * which the error recovery mode should be entered.
     */
    struct Error_decl : Decl {
        explicit Error_decl(const Source_location& sl) : Decl(Kind::Error, "", Storage_class::None, sl) {}
    };

    /**
     * A single block item, either a statement or a declaration.
     */
    struct Block_item {
        enum class Kind {Stmt, Decl} kind; ///< kind of block item
        union {
            Stmt* stmt; ///< pointer to the actual item, in this case a statement
            Decl* decl; ///< pointer to the actual item, in this case a declaration
        };

        explicit Block_item(Stmt* s) : kind(Kind::Stmt), stmt(s) {}
        explicit Block_item(Decl* d) : kind(Kind::Decl), decl(d) {}
    };

    /**
     * A sequence of block items that are arena allocated.
     */
    struct Block {
        Block_item* items; ///< arena allocated array of block items
        size_t n_items;    ///< number of items in the block
        Block(Block_item* items, size_t n_items) : items(items), n_items(n_items) {}

        /**
         * Constructs and returns a span over all block items for e.g. range based for loops.
         * @return std::span over all block items
         */
        [[nodiscard]] std::span<Block_item const> item_span() const { return std::span(items, n_items); }
    };

    /**
     * A return statement, holding a pointer to the expression to be returned.
     */
    struct Return : Stmt {
        Expr* expr; ///< expression to be returned
        Return(Expr* expr, const Source_location& sl) : Stmt(Kind::Return, sl), expr(expr) {}
    };

    /**
     * An if statement that keeps a pointer to the condition that dictates which branch is taken and a pointer to the
     * branch that is taken if the condition is true and the branch if the condition is false.
     */
    struct If : Stmt {
        Expr* condition; ///< condition that decides which branch to take
        Stmt* then;      ///< branch taken if condition is true
        Stmt* else_;     ///< branch taken if condition is false
        If(Expr* condition, Stmt* then, Stmt* else_, const Source_location& sl)
            : Stmt(Kind::If, sl), condition(condition), then(then), else_(else_) {}
    };

    /**
     * A compound statement that keeps a pointer to the block of its instructions.
     */
    struct Compound : Stmt {
        Block* block; ///< pointer to the block containing the block items
        Compound(Block* block, const Source_location& sl) : Stmt(Kind::Compound, sl), block(block) {}
    };

    /**
     * A break statement inside a loop or a switch.
     */
    struct Break : Stmt {
        std::string_view tag; ///< tag to the switch or loop that the break statement refers to
        Break(const std::string_view sv, const Source_location& sl) : Stmt(Kind::Break, sl), tag(sv) {}
    };

    /**
     * A continue statement inside a loop.
     */
    struct Continue : Stmt {
        std::string_view tag; ///< tag to the loop that the continue statement refers to
        Continue(const std::string_view sv, const Source_location& sl) : Stmt(Kind::Continue, sl), tag(sv) {}
    };

    /**
     * A while loop statement.
     */
    struct While : Stmt {
        Expr* condition;      ///< while the condition is true, the loop continues
        Stmt* body;           ///< pointer to the loop body
        std::string_view tag; ///< tag of the while loop to resolve break/continue statements
        While(Expr* c, Stmt* b, const std::string_view t, const Source_location& sl)
            : Stmt(Kind::While, sl), condition(c), body(b), tag(t) {}
    };

    /**
     * A do while loop statement.
     */
    struct Do_while : Stmt {
        Expr* condition;      ///< while the condition is true, the loop continues
        Stmt* body;           ///< pointer to the loop body
        std::string_view tag; ///< tag of the do while loop to resolve break/continue statements
        Do_while(Expr* c, Stmt* b, std::string_view t, const Source_location& sl)
            : Stmt(Kind::Do_while, sl), condition(c), body(b), tag(t) {}
    };

    /**
     * A null statement.
     */
    struct Null : Stmt {
        explicit Null(const Source_location& sl) : Stmt(Kind::Null, sl) {}
    };

    /**
     * Any labeled statement (not including case and default labels in switch statements).
     */
    struct Labeled : Stmt {
        std::string_view label; ///< identifier of the label
        Stmt* stmt;             ///< pointer to the statement that is labeled
        Labeled(const std::string_view l, Stmt* s, const Source_location& sl)
            : Stmt(Kind::Labeled, sl), label(l), stmt(s) {}
    };

    /**
     * A goto statement.
     */
    struct Goto : Stmt {
        std::string_view label; ///< identifier of the label so jump to
        Goto(const std::string_view s, const Source_location& sl) : Stmt(Kind::Goto, sl), label(s) {}
    };

    /**
     * A switch statement.
     */
    struct Switch : Stmt {
        bool has_default;     ///< true if the switch contains a default case
        int* cases;           ///< array of integers that have a case in the switch
        size_t n_cases;       ///< number of cases (not including the default case)
        Expr* expr;           ///< expression that is investigated, must be an integer expression
        Stmt* body;           ///< pointer to the body of the switch statement
        std::string_view tag; ///< unique tag of the switch to resolve break statements
        Switch(const bool hd, int* cs, const size_t n_cs, Expr* e, Stmt* s, const std::string_view t, const Source_location& sl)
            : Stmt(Kind::Switch, sl), has_default(hd), cases(cs), n_cases(n_cs), expr(e), body(s), tag(t) {}

        /**
         * Constructs and returns a span over all cases for e.g. range based for loops.
         * @return std::span over all cases
         */
        [[nodiscard]] std::span<int const> case_nums() const { return std::span(cases, n_cases); }
    };

    /**
     * A case or default label inside a switch statement.
     */
    struct Switch_label : Stmt {
        bool is_default;      ///< true for a default label, false for case labels
        int value;            ///< case value, only used if not a default label
        Stmt* stmt;           ///< pointer to the labeled statement
        std::string_view tag; ///< tag to its parent switch statement

        /// constructor for a case label
        Switch_label(const int v, Stmt* s, const std::string_view t, const Source_location& sl)
            : Stmt(Kind::Switch_label, sl), is_default(false), value(v), stmt(s), tag(t) {}

        /// constructor for a default label
        Switch_label(Stmt* s, const std::string_view t, const Source_location& sl)
            : Stmt(Kind::Switch_label, sl), is_default(true), value(0), stmt(s), tag(t) {}
    };

    // TODO: comment
    struct Expr_stmt : Stmt {
        Expr* expr;
        Expr_stmt(Expr* e, const Source_location& sl) : Stmt(Kind::Expr, sl), expr(e) {}
    };

    /**
     * Error statement, which is returned when parsing a statement results in a catastrophic error after which the
     * error recovery mode should be entered.
     */
    struct Error_stmt : Stmt {
        explicit Error_stmt(const Source_location& sl) : Stmt(Kind::Error, sl) {}
    };

    /**
     * A variable declaration node. Initializer is optional, should be a nullptr if it is absent.
     */
    struct Var_decl : Decl {
        Expr* init; ///< optional pointer to initializer expression (nullptr if absent)
        Var_decl(const std::string_view n, const Storage_class sc, Expr* e, const Source_location& sl)
            : Decl(Kind::Var, n, sc, sl), init(e) {}
    };

    /**
     * A function declaration node. The body is optional for forward declarations (no definition) an should be a nullptr
     * in this case. Function parameters are a arena allocated array of Var_decl pointers.
     */
    struct Func_decl : Decl {
        Var_decl** params; ///< arena allocated array of parameter declaration pointer
        size_t n_params;   ///< number of function parameters
        Block* body;       ///< pointer to the function body
        Func_decl(const std::string_view n, const Storage_class sc, Var_decl** p, const size_t np, Block* b, const Source_location& sl)
            : Decl(Kind::Func, n, sc, sl), params(p), n_params(np), body(b) {}
        /**
         * Returns a span over all parameter declarations for range-based for loops.
         * @return std::span over all parameter declaration pointers
         */
        [[nodiscard]] std::span<Var_decl*> parameters() const { return std::span(params, n_params); }
    };

    /**
     * The initialization clause of a for loop, either a declaration, an expression or absent.
     */
    struct For_init {
        enum class Kind {None, Decl, Expr} kind; ///< kind of the initialization clause
        union {
            Var_decl* var_decl; ///< pointer to the init, in this case a declaration
            Expr* expr;         ///< pointer to the init, in this case an expression
        };
        For_init() : kind(Kind::None), var_decl(nullptr) {}
        explicit For_init(Var_decl* d) : kind(Kind::Decl), var_decl(d) {}
        explicit For_init(Expr* e) : kind(Kind::Expr), expr(e) {}
    };

    /**
     * A for loop statement.
     */
    struct For : Stmt {
        For_init init;        ///< initialization fo the for loop variable
        Expr* condition;      ///< while the condition is true, the loop continues
        Expr* post;           ///< expression to execute after condition is checked to be true
        Stmt* body;           ///< pointer to the loop body
        std::string_view tag; ///< tag of the for loop to resolve break/continue statements
        For(const For_init fi, Expr* c, Expr* p, Stmt* b, const std::string_view t, const Source_location& sl)
            : Stmt(Kind::For, sl), init(fi), condition(c), post(p), body(b), tag(t) {}
    };

    /**
     * The root node of the AST, representing a complete translation unit. Contains arena allocated
     * array of top-level declarations.
     */
    struct Program {
        Decl** decls;   ///< arena allocated array of top-level constructs
        size_t n_decls; ///< number of top level constructs
        Program(Decl** d, const size_t nd) : decls(d), n_decls(nd) {}

        /**
         * Constructs and returns a span over all top-level declarations for e.g. range based for loops.
         * @return std::span over all top-level declarations
         */
        [[nodiscard]] std::span<Decl* const> declarations() const { return std::span(decls, n_decls); }
    };
}

#endif //UNTITLED_AST_HPP
