//
// Created by dominik on 7/31/25.
//

#include "parser.hpp"

/**
* Function to parse an argument list with the following syntax in EBNF:
* <arg-list> ::= <exp> { "," <exp> }
* @return vector of arguments
*/
std::vector<ast::Expression_ptr> Parser::parse_argument_list() {
    std::vector<ast::Expression_ptr> args;
    args.emplace_back(parse_expression(0));
    while (la_type == Token_type::Comma) {
        scan();
        args.emplace_back(parse_expression(0));
    }
    return args;
}

/**
* Function to parse a factor with the following syntax in EBNF:
* <factor> ::= <int> | <unop> <factor> | "(" <exp> ")" | <identifier>
*            | <identifier> "(" [ <arg-list> ] ")"
* <arg-list> ::= <exp> { "," <exp> }
* <exp> ::= <factor> | <exp> <binop> <exp>
* <unop> ::= "-" | "~" | "!"
* @return unique_ptr to a statement node
*/
ast::Expression_ptr Parser::parse_factor() {
    ast::Expression_ptr factor = std::monostate{};

    // check if prefix is present
    Token_type prefix = Token_type::None;
    if (la_type == Token_type::Increment) { prefix = Token_type::Increment; scan(); }
    else if (la_type == Token_type::Decrement) { prefix = Token_type::Decrement; scan(); }

    // integer constant
    if (la_type == Token_type::Number_constant) {
        scan();
        factor = std::make_unique<ast::Constant>(std::stoi(current.val));


    // identifier
    // this needs some caution because of type checking and general semantic analysis
    } else if (la_type == Token_type::Identifier) {
        scan();
        const std::string name = current.val;


        // function call
        if (la_type == Token_type::Open_parenthesis) {
            // check if function is in symbol table
            const auto symbol = current_scope->lookup(name);
            // not declared
            if (symbol == nullptr) {
                std::string msg = "with name " + name;
                diag.report_issue(Severity::Error, current, Error_kind::Undeclared_function, msg);
            // check that is actually is a function and not a variable etc.
            } else {
                if (symbol->kind != Symbol::Kind::Func) {
                    std::string msg = "with name " + name;
                    diag.report_issue(Severity::Error, current, Error_kind::Undeclared_function, msg);
                }
            }

            scan(); // '(' token

            // argument list empty
            if (la_type == Token_type::Close_parenthesis) {
                scan();
                factor = std::make_unique<ast::Function_call>(name, std::vector<ast::Expression_ptr>{});
            // non-empty argument list
            } else {
                auto arglist = parse_argument_list();
                //expect(Token_type::Close_parenthesis, Error_kind::Missing_closing_parenthesis, "at end of argument list in function call");
                if (la_type != Token_type::Close_parenthesis) {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_closing_parenthesis, "at end of argument list in function call");
                    return ast::ErrorExpr_ptr();
                } else scan();

                // type check argument
                if (symbol != nullptr)
                if (arglist.size() != symbol->type->params.size()) {
                    std::string msg = "got " + std::to_string(arglist.size()) + " but expected " + std::to_string(symbol->type->params.size());
                    diag.report_issue(Severity::Error, current, Error_kind::Wrong_number_of_arguments, msg);
                    msg = "with name " + name;
                    diag.report_issue(Severity::Note, get_location(symbol), Error_kind::First_defined_here, msg);
                }
                // TODO: each argument will actually have to be checked, so far only integers are implemented...

                factor = std::make_unique<ast::Function_call>(name, std::move(arglist));
            }


        // variable
        } else {
            auto iden = std::make_unique<ast::Variable>(name);

            // check if variable is declared
            const auto symbol = current_scope->lookup(iden->name);
            // not declared
            if (symbol == nullptr) {
                std::string msg = "with name " + iden->name;
                diag.report_issue(Severity::Error, current, Error_kind::Undeclared_symbol, msg);

            // use mangled name in AST if it's a variable
            } else {
                if (symbol->kind != Symbol::Kind::Var) {
                    diag.report_issue(Severity::Error, current, Error_kind::Func_used_as_var, "");
                    diag.report_issue(Severity::Note, get_location(symbol), Error_kind::First_defined_here, "");
                } else iden->name = symbol->unique_name;
            }

            factor = std::move(iden);
        } // end of variable section

    // unary operator
    } else if (look_ahead.is_unary()) {
        scan();
        switch (current.type) {
            case Token_type::Minus:
                factor = std::make_unique< ast::Unary>(ast::Unary_operator::Negate, parse_factor()); break;
            case Token_type::Complement:
                factor = std::make_unique< ast::Unary>(ast::Unary_operator::Complement, parse_factor()); break;
            case Token_type::Not:
                factor = std::make_unique< ast::Unary>(ast::Unary_operator::Not, parse_factor()); break;
            default:
                diag.report_issue(Severity::Error, current, Error_kind::Unknown, " unary operator");
        }

    // expression in parentheses
    } else if (la_type == Token_type::Open_parenthesis) {
        scan();
        factor = parse_expression(0);
        if (la_type != Token_type::Close_parenthesis) {
            report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_closing_parenthesis, "after expression");
            return ast::ErrorExpr_ptr();
        } else scan();

    // error
    } else {
        report_syntax_error(Severity::Error, look_ahead, Error_kind::Invalid_expression, "");
        return ast::ErrorExpr_ptr(); // TODO: check that this line does not enable infinite loops etc.
        scan(); // go to next token, otherwise an infinite loop is possible if no beginning matches with an expression
        // check if end of file was reached
        if (current.type == Token_type::EOF_) {
            diag.report_issue(Severity::Error, current, Error_kind::Reached_eof, "in expression");
        }
    }

    // check for suffixes
    // increment
    if (match(Token_type::Increment)) {
        if (not ast::is_lvalue(factor)) diag.report_issue(Severity::Error, current, Error_kind::Invalid_lvalue, " in increment");
        factor = std::make_unique<ast::Post_increment>(ast::Post_increment(std::move(factor)));
    }
    // decrement
    else if (match(Token_type::Decrement)) {
        if (not ast::is_lvalue(factor)) diag.report_issue(Severity::Error, current, Error_kind::Invalid_lvalue, " in decrement");
        factor = std::make_unique<ast::Post_decrement>(ast::Post_decrement(std::move(factor)));
    }

    // apply prefix (if present) before return
    if (prefix != Token_type::None)
        if (not ast::is_lvalue(factor)) diag.report_issue(Severity::Error, current, Error_kind::Invalid_lvalue, " after prefix");
    switch (prefix) {
        case Token_type::Increment: factor = std::make_unique<ast::Pre_increment>(ast::Pre_increment(std::move(factor))); break;
        case Token_type::Decrement: factor = std::make_unique<ast::Pre_decrement>(ast::Pre_decrement(std::move(factor))); break;
        default: break;
    }

    if (std::holds_alternative<std::monostate>(factor)) scan();
    return factor;
}
