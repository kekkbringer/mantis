//
// Created by dominik on 7/7/25.
//

#include "parser.hpp"

/**
 * Function to parse a block with the following syntax in EBNF:
 * <block> ::= "{" { <block-item> } "}"
 * <block-item> ::= <statement> | <declaration>
 * @return unique_ptr to a declaration node
 */
ast::Block Parser::parse_block(const bool new_scope) {
    ast::Block block;

    expect(Token_type::Open_brace, Error_kind::Missing_open_brace, "at start of block");

    // enter new scope
    if (new_scope) enter_new_scope();

    while (la_type != Token_type::Close_brace and la_type != Token_type::EOF_) {
        // declarations always start with a specifier
        if (look_ahead.is_specifier())
            block.emplace_back(std::move(parse_declaration()));

        // otherwise it is a statement
        else {
            // in the beginning, check if the compiler reached end of file by continuously encountering invalid or unexpected
            // tokens or got otherwise stuck in an endless loop
            if (current.type == Token_type::EOF_) {
                diag.report_issue(Severity::Error, current, Error_kind::Reached_eof, "while parsing block statement");
                return block;
            }
            block.emplace_back(std::move(parse_statement()));
        }
    }

    expect(Token_type::Close_brace, Error_kind::Missing_closing_brace, "at end of block");

    // leave scope of block again
    if (new_scope) leave_scope();

    return block;
}
