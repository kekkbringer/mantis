//
// Created by dominik on 7/4/25.
//

#include "parser.hpp"

/**
 * Function to parse a C program with the following syntax in EBNF:
 * <program> ::= { <declaration> }
 * @return program AST node
 */
ast::Program Parser::parse_program() {
    ast::Program prog;

    // global scope already exists, got passed to parser on construction
    // -> no need to enter a new scope here

    // expect a list of declarations of whatever length
    // <declaration> ::= <variable-declaration> | <function-declaration>
    // <variable-declaration> ::= { <specifier> }+ <identifier> [ "=" <exp> ] ";"
    // <function-declaration> ::= { <specifier> }+ <identifier> "(" <param-list> ")" ( <block> | ";" )
    //while (look_ahead.is_specifier()) {
    while (la_type != Token_type::EOF_) {
        ast::Declaration_ptr decl = parse_declaration();
        prog.declarations.emplace_back(std::move(decl));
    }

    // check if there is junk remaining in the file
    if (la_type != Token_type::EOF_) {
        diag.report_issue(Severity::Error, look_ahead, Error_kind::Reached_eof, "but expected declaration as top level construct");
    }

    return prog;
}