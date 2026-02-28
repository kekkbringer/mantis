//
// Created by dominik on 7/4/25.
//

#ifndef AST_OLD_HPP
#define AST_OLD_HPP

#include <utility>
#include <variant>
#include <memory>
#include <set>
#include <vector>

#include "ast_unary_operator.hpp"
#include "ast_binary_operator.hpp"
#include "source_location.hpp"

/**
 * Holds all classes that make up the abstract syntax tree structure.
 */
namespace ast {
    // forward declarations of everything
    class Program;
    class Function_declaration; using Function_declaration_ptr = std::unique_ptr<Function_declaration>;
    class Variable_declaration; using Variable_declaration_ptr = std::unique_ptr<Variable_declaration>;
    class ErrorDecl;            using ErrorDecl_ptr            = std::unique_ptr<ErrorDecl>;
    using Declaration_ptr = std::variant<std::monostate, ErrorDecl_ptr,
                                         Function_declaration_ptr, Variable_declaration_ptr>;

    /**
     * A non-owning reference to a declaration, used in the symbol table to refer back to the declaration. This is
     * needed in the resolution of some identifiers.
     */
    using Decl_view = std::variant<std::monostate*, Function_declaration*, Variable_declaration*>;

    enum class Storage_class { None, Static, Extern };

    // expressions
    class Constant;       using Constant_ptr       = std::unique_ptr<Constant>;
    class Variable;       using Variable_ptr       = std::unique_ptr<Variable>;
    class Unary;          using Unary_ptr          = std::unique_ptr<Unary>;
    class Binary;         using Binary_ptr         = std::unique_ptr<Binary>;
    class Assignment;     using Assignment_ptr     = std::unique_ptr<Assignment>;
    class Conditional;    using Conditional_ptr    = std::unique_ptr<Conditional>;
    class Function_call;  using Function_call_ptr  = std::unique_ptr<Function_call>;
    class Post_increment; using Post_increment_ptr = std::unique_ptr<Post_increment>;
    class Pre_increment;  using Pre_increment_ptr  = std::unique_ptr<Pre_increment>;
    class Post_decrement; using Post_decrement_ptr = std::unique_ptr<Post_decrement>;
    class Pre_decrement;  using Pre_decrement_ptr  = std::unique_ptr<Pre_decrement>;
    class ErrorExpr;      using ErrorExpr_ptr      = std::unique_ptr<ErrorExpr>;
    using Expression_ptr = std::variant<std::monostate, Constant_ptr, Variable_ptr, Unary_ptr, Binary_ptr,
                                        Assignment_ptr, Conditional_ptr, Function_call_ptr,
                                        Post_increment_ptr, Post_decrement_ptr, Pre_increment_ptr, Pre_decrement_ptr,
                                        ErrorExpr_ptr>;

    /**
     * Returns 'true' if the expression pointed to by 'expptr' is a valid lvalue, 'false' otherwise
     * @param expptr point to the expression in question
     * @return 'true' if expression is valid lvalue
     */
    inline bool is_lvalue(const Expression_ptr& expptr) {
        return std::holds_alternative<Variable_ptr>(expptr);
    }

    // statements
    class Return;       using Return_ptr    = std::unique_ptr<Return>;
    class If;           using If_ptr        = std::unique_ptr<If>;
    class Compound;     using Compound_ptr  = std::unique_ptr<Compound>;
    class Break;        using Break_ptr     = std::unique_ptr<Break>;
    class Continue;     using Continue_ptr  = std::unique_ptr<Continue>;
    class While;        using While_ptr     = std::unique_ptr<While>;
    class Do_while;     using Do_while_ptr  = std::unique_ptr<Do_while>;
    class For;          using For_ptr       = std::unique_ptr<For>;
    class Null {};      using Null_ptr      = std::unique_ptr<Null>;
    class Labeled;      using Labeled_ptr   = std::unique_ptr<Labeled>;
    class Goto;         using Goto_ptr      = std::unique_ptr<Goto>;
    class Switch;       using Switch_ptr    = std::unique_ptr<Switch>;
    class Case;         using Case_ptr      = std::unique_ptr<Case>;
    class Default;      using Default_ptr   = std::unique_ptr<Default>;
    class ErrorStmt;    using ErrorStmt_ptr = std::unique_ptr<ErrorStmt>;
    using Statement_ptr = std::variant<std::monostate, Return_ptr, If_ptr, Compound_ptr, Break_ptr, Continue_ptr,
                                       While_ptr, Do_while_ptr, For_ptr, Null_ptr, Labeled_ptr, Goto_ptr, Expression_ptr,
                                       Switch_ptr, Case_ptr, Default_ptr, ErrorStmt_ptr>;

    using Block_item_ptr = std::variant<std::monostate, Statement_ptr, Declaration_ptr>;
    using Block = std::vector<Block_item_ptr>;
    using Block_ptr = std::unique_ptr<Block>;

    using For_init = std::variant<std::monostate, Declaration_ptr, Expression_ptr>;


    // actual class implementations follow
    /**
     * AST program that contains a list of declarations.
     */
    class Program {
    public:
        std::vector<Declaration_ptr> declarations; ///< list of declarations that make up the program
        Program() = default;
    };

    /**
     * AST variable declaration, one of the two different declarations (next to a function declaration). Declarations
     * are the only valid top level construct of the AST program.
     */
    class Variable_declaration {
    public:
        std::string name;                       ///< name of the variable
        Expression_ptr init = std::monostate{}; ///< (potentially) the expression to initiate the variable with
        Storage_class storage_class;            ///< storage class of the declared variable
        Source_location loc;                    ///< location in the source code

        Variable_declaration(std::string n, Expression_ptr i, const Storage_class& st) : name(std::move(n)), init(std::move(i)), storage_class(st) {}
        Variable_declaration(std::string n, const Storage_class& st) : name(std::move(n)), storage_class(st) {}
    };

    /**
     * AST function declaration, one of the two different declaration (next to a variable declaration). Declarations are
     * the only valid top level construct of the AST program.
     */
    class Function_declaration {
    public:
        std::string name;                                  ///< name of the function
        std::vector<Variable_declaration_ptr> params;      ///< parameter of the function
        Block_ptr body;                                    ///< body of the function, containing different block items
        Storage_class storage_class = Storage_class::None; ///< storage class of the declared variable
        Source_location loc;                               ///< location in the source code

        Function_declaration(std::string n, const Storage_class& sc) : name(std::move(n)), storage_class(sc) {}
        Function_declaration(std::string n, std::vector<Variable_declaration_ptr>& pl, const Storage_class& sc) : name(std::move(n)), params(std::move(pl)), storage_class(sc) {}
        Function_declaration(std::string n, std::vector<Variable_declaration_ptr>& pl, Block_ptr& b, const Storage_class& sc) : name(std::move(n)), params(std::move(pl)), body(std::move(b)), storage_class(sc) {}
        Function_declaration(std::string n, std::vector<Variable_declaration_ptr>& pl, Block& b, const Storage_class& sc) : name(std::move(n)), params(std::move(pl)), body(std::make_unique<Block>(std::move(b))), storage_class(sc) {}
    };

    /**
     * AST node that represents a switch statement.
     */
    class Switch {
    public:
        bool has_default = false; ///< indicates if a default label in contained in the switch statement
        std::set<int> cases;      ///< set of all cases contained in the switch statement
        Expression_ptr expr;      ///< pointer to the expression that is investigated, must be a constexpr
        Statement_ptr body;       ///< pointer to the body of the switch statement, contains all cases etc.
        std::string tag;          ///< unique tag of the switch for 'break' resolution
    };

    /**
     * AST node for a 'case' label inside a 'switch'.
     */
    class Case {
    public:
        int num;            ///< take this case if the expression in switch is equal to 'num'
        std::string tag;    ///< tag to its parent switch statement
        Statement_ptr stmt; ///< pointer to the statement labeled with this case label
    };

    /**
     * AST node for a 'default' label inside a 'switch'.
     */
    class Default {
    public:
        std::string tag;    ///< tag to its parent switch statement
        Statement_ptr stmt; ///< pointer to the statement labeled with this case label
    };


    /**
     * AST node for a 'break' statement inside a loop or a switch.
     */
    class Break {
    public:
        std::string tag; ///< tag to the switch or loop that the 'break' statement refers to
    };

    /**
     * AST node for a 'continue' statement inside a loop.
     */
    class Continue {
    public:
        std::string tag; ///< tag to the loop that the 'continue' statement refers to
    };

    /**
     * AST node for a label of a statement.
     */
    class Labeled {
    public:
        std::string label;  ///< identifier of the label
        Statement_ptr stmt; ///< pointer to the statement that's labeled
    };


    /**
     * AST node for a 'goto' statement.
     */
    class Goto {
    public:
        std::string label; ///< identifier of the label to go to
    };

    /**
     * AST return statement.
     * The return statement transfers control of a function back to the caller. 'expr' will hold the expression to be
     * returned by the function.
     */
    class Return {
    public:
        Expression_ptr expr; ///< expression to be returned
        explicit Return(Expression_ptr expr) : expr(std::move(expr)) {}
    };

    /**
     * AST compound statement node.
     */
    class Compound {
    public:
        Block_ptr block; ///< pointer to the block containing the block items that make up the compound statement
    };

    /**
     * AST if statement node.
     */
    class If {
    public:
        Expression_ptr condition; ///< condition of the if statement
        Statement_ptr then;       ///< statement to be taken if condition is true
        Statement_ptr else_;      ///< statement to be taken if condition is false
    };

    /**
     * AST while loop statement node.
     */
    class While {
    public:
        Expression_ptr condition; ///< while the condition is true, the loop continues
        Statement_ptr body;       ///< body of the loop to be executed each iteration
        std::string tag;          ///< tag of the while loop to resolve 'break'/'continue'
    };

    /**
     * AST do while loop statement node.
     */
    class Do_while {
    public:
        Statement_ptr body; ///< body of the loop to be executed each iteration
        Expression_ptr condition; ///< while the condition is true, the loop continues
        std::string tag;          ///< tag of the do while loop to resolve 'break'/'continue'
    };

    /**
     * AST for loop statement node.
     */
    class For {
    public:
        For_init init;            ///< initialization of the for loop variable (optional)
        Expression_ptr condition; ///< while the condition is true, the loop continues
        Expression_ptr post;      ///< expression to be executed after condition is checked to be true
        Statement_ptr body;       ///< body of the loop to be executed each iteration
        std::string tag;          ///< tag of the for loop to resolve 'break'/'continue'
    };

    /**
     * AST constant node.
     * So far, only integer constants are considered.
     */
    class Constant {
    public:
        int val; ///< value of the constant
        explicit Constant (const int v) : val(v) {}
    };

    /**
     * AST variable node.
     */
    class Variable {
    public:
        std::string name; ///< name of the variable

        explicit Variable (std::string n) : name(std::move(n)) {}

        // copy constructor for variable node is useful for compound statements
        Variable(Variable& other) = default;
    };

    /**
     * General AST unary operation
     */
    class Unary {
    public:
        Unary_operator op;   ///< type of the unary operation
        Expression_ptr expr; ///< expression that the unary operation acts on
    };

    /**
     * General AST binary operation
     */
    class Binary {
    public:
        Binary_operator op; ///< type of the binary operation
        Expression_ptr lhs; ///< left-hand side the operation acts on
        Expression_ptr rhs; ///< right-hand side the operation acts on
    };

    /**
     * AST assignment node.
     * Assigns the right-hand side to the left-hand side.
     */
    class Assignment {
    public:
        Expression_ptr lhs; ///< target of the assignment
        Expression_ptr rhs; ///< expression that will be assigned to the left-hand side
    };

    /**
     * AST function call node.
     */
    class Function_call {
    public:
        std::string name;                 ///< name of the function to be called
        std::vector<Expression_ptr> args; ///< arguments that are passed to the called function
    };

    /**
     * AST conditional statement node, the ternary construct like e.g.: a == b ? "yes" : "no"
     * If the condition is true, it evaluates to the left-hand side (lhs), otherwise to the right-hand side (rhs).
     */
    class Conditional {
    public:
        Expression_ptr condition; ///< condition to be checked
        Expression_ptr lhs;       ///< left-hand side, to be used if condition is true
        Expression_ptr rhs;       ///< right-hand side, to be used if condition is false
    };

    /**
     * AST node for a 'post increment' expression, e.g. 'i++'.
     */
    class Post_increment {
    public:
        Expression_ptr expr; ///< pointer to the expression the increment applies to, hopefully a variable.
    };

    /**
     * AST node for a 'pre increment' expression, e.g. '++i'.
     */
    class Pre_increment {
    public:
        Expression_ptr expr; ///< pointer to the expression the increment applies to, hopefully a variable.
    };

    /**
     * AST node for a 'post decrement' expression, e.g. 'i--'.
     */
    class Post_decrement {
    public:
        Expression_ptr expr; ///< pointer to the expression the decrement applies to, hopefully a variable.
    };

    /**
     * AST node for a 'pre decrement' expression, e.g. '--i'.
     */
    class Pre_decrement {
    public:
        Expression_ptr expr; ///< pointer to the expression the decrement applies to, hopefully a variable.
    };

    /**
     * Dummy node that gets emitted when a catastrophic error is encountered during parsing.
     */
    class ErrorStmt {
    };

    /**
     * Dummy node that gets emitted when a catastrophic error is encountered during parsing.
     */
    class ErrorExpr {
    };

    /**
     * Dummy node that gets emitted when a catastrophic error is encountered during parsing.
     */
    class ErrorDecl{
    };

}

#endif //AST_OLD_HPP
