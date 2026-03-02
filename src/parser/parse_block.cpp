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
ast::Block* Parser::parse_block(const bool new_scope) {
    expect(Token_type::Open_brace, Error_kind::Missing_open_brace, "at start of block");

    // enter new scope
    if (new_scope) enter_new_scope();

    // collect block items in temporary vector until we know total number of items
    std::vector<ast::Block_item> temp;
    while (la_type != Token_type::Close_brace and la_type != Token_type::EOF_) {
        // declarations always start with a specifier
        if (look_ahead.is_specifier()) {
            ast::Decl* decl = parse_declaration();
            temp.emplace_back(decl);
        }

        // otherwise it is a statement
        else {
            // check if the compiler reached end of file by continuously encountering invalid or unexpected
            // tokens or got otherwise stuck in an endless loop
            if (current.type == Token_type::EOF_) {
                diag.report_issue(Severity::Error, current, Error_kind::Reached_eof, "while parsing block statement");
                break;
            }
            ast::Stmt* stmt = parse_statement();
            temp.emplace_back(stmt);
        }
    }

    expect(Token_type::Close_brace, Error_kind::Missing_closing_brace, "at end of block");

    // leave scope of block again
    if (new_scope) leave_scope();

    // copy collected items into arena and construct block node
    // only pointers are copied here
    auto* items = arena->allocate_array<ast::Block_item>(temp.size());
    std::ranges::copy(temp, items);
    return arena->allocate<ast::Block>(items, temp.size());
}
