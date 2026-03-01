//
// Created by dominik on 7/7/25.
//

#include <ranges>

#include "asm_nodes.hpp"
#include "parser.hpp"

/**
 * This function is called after a catastrophic error, usually a non-trivial syntax error, is encountered to
 * synchronize the incoming token stream to a safe reentry point. The save point should represent a place where parsing
 * a statement can be started again.
 */
void Parser::synchronize_stmt() {
    scan(); // consume bad token, prevent infinite loops

    while (la_type != Token_type::EOF_) {
        // reentry after a semicolon should always be safe
        if (current.type == Token_type::Semicolon) {
            in_error_recovery = false;
            return;
        }

        // check for initial tokens of statements
        switch (current.type) {
            case Token_type::if_:
            case Token_type::do_:
            case Token_type::while_:
            case Token_type::for_:
            case Token_type::return_:
            case Token_type::Open_brace:
            case Token_type::int_:
            case Token_type::void_:
                in_error_recovery = false;
                return;

            case Token_type::Close_brace:
                // might be a special case because you could get to the file scope here
                in_error_recovery = false;
                return;

            default:;
        }

        // token was not safe, continue looking
        scan();
    }

    // if we reach end of file we can just leave error recovery mode
    in_error_recovery = false;
}

/**
 * This is the wrapper routine for the actual statement parsing function. This wrapper handles recovery from error mode
 * after a catastrophic error was encountered by resynchronizing the incoming token stream to a safe reentry point. The
 * parsing routine indicates a catastrophic error by returning an ErrorStmt. In this case we try to recover and return a
 * pointer to an ErrorStmt node ourselves.
 * @return
 */
ast::Stmt* Parser::parse_statement() {
    // if we are already in error recovery mode -> synchronize
    if (in_error_recovery) {
        synchronize_stmt();
        return arena->allocate<ast::Error_stmt>(Source_location());
    }

    // call actual parsing function
    const auto stmt_ptr = parse_statement_inner();

    // check if catastrophic error was encountered
    if (stmt_ptr->kind == ast::Stmt::Kind::Error) {
        synchronize_stmt();
        return stmt_ptr;
    }

    return stmt_ptr;
}

/**
 * Parses an initialization in a for loop header
 * <for-init> ::= <declaration> | [ <exp> ] ";"
 * @return for init
 */
ast::For_init Parser::parse_for_init() {
    // variable declaration
    if (look_ahead.is_specifier()) {
        // var init in for loop my not be 'static' or 'extern'
        if (la_type == Token_type::static_ or la_type == Token_type::extern_) {
            diag.report_issue(Severity::Error, current, Error_kind::Invalid_storage_class,
                                                    ", 'static' in for loop init not legal");
        }

        auto decl = parse_declaration();
        if (decl->kind != ast::Decl::Kind::Var) {
            diag.report_issue(Severity::Error, current, Error_kind::Expected_statement, "in for loop initialization");
            return ast::For_init();
        } else {
            const auto var_decl = cast<ast::Var_decl>(decl);
            return ast::For_init(var_decl);
        }

    // empty
    } else if (la_type == Token_type::Semicolon) {
        scan();
        return ast::For_init();

    // expression
    } else {
        auto expr = parse_expression(0);
        if (expr->kind == ast::Expr::Kind::Error) return ast::For_init();
        if (la_type != Token_type::Semicolon) {
            report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_semicolon, "after expression in for loop initialization");
            return ast::For_init();
        } else scan();
        return ast::For_init(expr);
    }
}

/**
* Note: This function contains the actual parsing logic but should be called only from within its wrapper function
* for error recovery by synchronization. If this function encounters a catastrophic error and wishes to enter error
* recovery mode, it returns an ErrorStmt_ptr to indicate its intent to the wrapper.
* Function to parse a statement with the following syntax in EBNF:
* <statement> ::= "return" <exp> ";" | <exp> ";" | ";"
*               | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
*               | <label> ":" <statement>
*               | <block>
* @return unique_ptr to a statement node
*/
ast::Stmt* Parser::parse_statement_inner() {
    const auto loc = current.location;

    switch (la_type) {
        // return statement
        case Token_type::return_: {
            scan(); // return token
            auto expr = parse_expression(0);
            if (expr->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(Source_location());
            if (la_type != Token_type::Semicolon) {
                // check if it's just a stray token
                if (lexer.peek(0).type == Token_type::Semicolon) {
                    // try to keep on parsing
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in return statement");
                    scan(); scan();
                // not just a stray token -> enter recovery mode
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_semicolon, "at end of return statement");
                    return arena->allocate<ast::Error_stmt>(Source_location());
                }
            } else scan();
            return arena->allocate<ast::Return>(expr, loc);
        }

        // case label
        case Token_type::case_: {
            scan(); // 'case' token

            // fetch tag of last switch
            std::string tag;
            for (const auto & it : std::ranges::reverse_view(control_stack)) {
                if (it.kind == Control_target::Kind::Switch) { tag = it.tag; break; }
            }
            if (tag.empty()) diag.report_issue(Severity::Error, current, Error_kind::Case_outside_switch);

            int val;
            if (la_type != Token_type::Number_constant) {
                // check if it's just a stray token
                if (lexer.peek(0).type == Token_type::Number_constant) {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in case labeled statement");
                    scan(); scan();
                // not just a stray token -> enter recovery mode
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Case_without_case);
                    return arena->allocate<ast::Error_stmt>(Source_location());
                }

            } else {
                scan();
                val = std::stoi(current.val);
            }

            if (la_type != Token_type::Colon) {
                // check if it's just a stray token
                if (lexer.peek(0).type == Token_type::Colon) {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in case labeled statement");
                    scan(); scan();
                // not just a stray token -> enter recovery mode
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_colon, "after case label");
                    return arena->allocate<ast::Error_stmt>(Source_location());
                }
            } else scan();
            auto stmt = parse_statement();

            if (not current_switch_cases.empty()) {
                if (current_switch_cases.back().contains(val)) {
                    const std::string msg = "with value " + std::to_string(val);
                    diag.report_issue(Severity::Error, current, Error_kind::Duplicate_case, msg);
                }
                current_switch_cases.back().insert(val);
            }
            const auto tag_view = string_table->intern(".case." + std::to_string(val) + "." + tag);
            return arena->allocate<ast::Switch_label>(val, stmt, tag_view, loc);
        }

        // default label
        case Token_type::default_: {
            scan(); // 'default' token

            // fetch tag of last switch
            std::string tag;
            for (const auto & it : std::ranges::reverse_view(control_stack)) {
                if (it.kind == Control_target::Kind::Switch) { tag = it.tag; break; }
            }
            if (tag.empty()) diag.report_issue(Severity::Error, current, Error_kind::Default_outside_switch);

            expect(Token_type::Colon, Error_kind::Missing_colon, "after default keyword");
            auto stmt = parse_statement();

            if (not current_switch_cases.empty()) {
                if (current_switch_has_default.back())
                    diag.report_issue(Severity::Error, current, Error_kind::Duplicate_default, "");
                current_switch_has_default.back() = true;
            }
            const auto tag_view = string_table->intern(".default." + tag);
            return arena->allocate<ast::Switch_label>(stmt, tag_view, loc);
        }

        // switch statement
        case Token_type::switch_: {
            scan(); // 'switch' token
            expect(Token_type::Open_parenthesis, Error_kind::Missing_open_parenthesis, "after switch keyword");
            auto expr = parse_expression(0);
            if (expr->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);
            expect(Token_type::Close_parenthesis, Error_kind::Missing_closing_parenthesis, "after expression in switch statement");

            // tag switch and push to control target stack
            const std::string tag = "switch_" + std::to_string(mangle_counter++);
            control_stack.emplace_back(Control_target(Control_target::Kind::Switch, tag));

            // prepare new switch cases
            current_switch_cases.emplace_back();
            current_switch_has_default.emplace_back(false);

            // parse (compound) statement
            auto stmt = parse_statement();

            // transfer all cases to AST node
            const bool has_def = current_switch_has_default.back();
            const size_t n_case = current_switch_cases.back().size();

            int* cases = arena->allocate_array<int>(n_case);
            std::ranges::copy(current_switch_cases.back(), cases);
            const auto tag_view = string_table->intern(tag);
            auto switch_ptr = arena->allocate<ast::Switch>(has_def, cases, n_case, expr, stmt, tag_view, loc);

            current_switch_cases.pop_back();
            current_switch_has_default.pop_back();
            control_stack.pop_back();
            return switch_ptr;
        }


        // do while loop
        case Token_type::do_: {
            scan();

            // tag while loop and push to control target stack
            const std::string tag = "do_while_" + std::to_string(mangle_counter++);
            control_stack.emplace_back(Control_target(Control_target::Kind::Loop, tag));

            auto stmt = parse_statement();

            // pop it from the control target stack
            control_stack.pop_back();

            expect(Token_type::while_, Error_kind::Missing_while, "after do-while loop statement");
            expect(Token_type::Open_parenthesis, Error_kind::Missing_open_parenthesis, "after while keyword in do-while loop");

            auto cond = parse_expression(0);
            if (cond->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);

            expect(Token_type::Close_parenthesis, Error_kind::Missing_closing_parenthesis, "after statement in do-while loop");
            expect(Token_type::Semicolon, Error_kind::Missing_semicolon, "at end of do-while loop");

            const auto tag_view = string_table->intern(tag);
            return arena->allocate<ast::Do_while>(cond, stmt, tag_view, loc);
        }

        // while loop
        case Token_type::while_: {
            scan(); // 'while' token
            expect(Token_type::Open_parenthesis, Error_kind::Missing_open_parenthesis, "after while keyword");
            auto cond = parse_expression(0);
            if (cond->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);
            expect(Token_type::Close_parenthesis, Error_kind::Missing_closing_parenthesis, "after condition in while statement");

            // tag while loop and push to control target stack
            const std::string tag = "while_" + std::to_string(mangle_counter++);
            control_stack.emplace_back(Control_target(Control_target::Kind::Loop, tag));

            auto stmt = parse_statement();

            // pop it from the control target stack
            control_stack.pop_back();

            const auto tag_view = string_table->intern(tag);
            return arena->allocate<ast::While>(cond, stmt, tag_view, loc);
        }

        // for loop
        case Token_type::for_: {
            scan(); // 'for' token

            if (la_type != Token_type::Open_parenthesis) {
                // check if it's just a stray token
                if (lexer.peek(0).type == Token_type::Open_parenthesis) {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in for loop");
                    scan(); scan();
                // not just a stray token -> enter recovery mode
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_open_parenthesis, "at beginning of for loop header");
                    return arena->allocate<ast::Error_stmt>(loc);
                }
            } else scan();

            // for loop header introduces a new scope, tag for loop and push to control target stack
            enter_new_scope();
            const std::string tag = "for_" + std::to_string(mangle_counter++);
            control_stack.emplace_back(Control_target(Control_target::Kind::Loop, tag));

            // for init
            auto init = parse_for_init();

            // [ <exp> ] ";"
            ast::Expr* cond = nullptr;
            if (la_type == Token_type::Semicolon) scan();
            else {
                cond = parse_expression(0);
                if (cond->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);
                if (la_type != Token_type::Semicolon) {
                    // check if it's just a stray token
                    if (lexer.peek(0).type == Token_type::Semicolon) {
                        diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in for loop");
                        scan(); scan();
                    // not just a stray token -> enter recovery mode
                    } else {
                        report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_semicolon, "after condition in for loop header");
                        return arena->allocate<ast::Error_stmt>(loc);
                    }
                } else scan();
            }

            // [ <exp> ] ")"
            ast::Expr* post = nullptr;
            if (la_type != Token_type::Close_parenthesis) {
                post = parse_expression(0);
                if (post->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);
            }
            if (la_type != Token_type::Close_parenthesis) {
                // check if it's just a stray token
                if (lexer.peek(0).type == Token_type::Close_parenthesis) {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in for loop");
                    scan(); scan();
                // not just a stray token -> enter recovery mode
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_closing_parenthesis, "after for loop header");
                    return arena->allocate<ast::Error_stmt>(loc);
                }
            } else scan();

            // <statement>
            auto stmt = parse_statement();

            // leave scope introduces by for loop header and pop it from the control target stack
            leave_scope();
            control_stack.pop_back();

            const auto tag_view = string_table->intern(tag);
            return arena->allocate<ast::For>(init, cond, post, stmt, tag_view, loc);
        }

        // continue statement
        case Token_type::continue_: {
            scan();
            // fetch tag of last loop/switch
            std::string tag;
            for (const auto & it : std::ranges::reverse_view(control_stack)) {
                if (it.kind == Control_target::Kind::Loop) { tag = it.tag; break; }
            }
            if (tag.empty()) diag.report_issue(Severity::Error, current, Error_kind::Continue_outside_loop);
            expect(Token_type::Semicolon, Error_kind::Missing_semicolon, "after continue statement");
            const auto tag_view = string_table->intern(tag);
            return arena->allocate<ast::Continue>(tag_view, loc);
        }

        // break statement
        case Token_type::break_: {
            scan();
            // fetch tag of last loop/switch
            const std::string tag = (control_stack.empty() ? "" : control_stack.back().tag);
            if (tag.empty()) diag.report_issue(Severity::Error, current, Error_kind::Break_outside_control_target);
            expect(Token_type::Semicolon, Error_kind::Missing_semicolon, "after break statement");
            const auto tag_view = string_table->intern(tag);
            return arena->allocate<ast::Break>(tag_view, loc);
        }

        // block statement
        case Token_type::Open_brace: {
            ast::Block* block = parse_block(true); // true = enter new scope on block enter
            return arena->allocate<ast::Compound>(block, loc);
        }

        // if statement
        case Token_type::if_: {
            scan(); // "if" token
            expect(Token_type::Open_parenthesis, Error_kind::Missing_open_parenthesis, " after 'if'");
            auto cond = parse_expression(0);
            if (cond->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);
            expect(Token_type::Close_parenthesis, Error_kind::Missing_closing_parenthesis, " after condition in if-statement");
            auto then = parse_statement();
            if (then->kind == ast::Stmt::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);

            if (la_type == Token_type::else_) {
                scan(); // "else" token
                auto else_ = parse_statement();
                return arena->allocate<ast::If>(cond, then, else_, loc);
            } else {
                return arena->allocate<ast::If>(cond, then, nullptr, loc);
            }
        }

        // null statement
        case Token_type::Semicolon: {
            scan();
            return arena->allocate<ast::Null>(loc);
        }

        // goto statement
        case Token_type::goto_: {
            scan(); // 'goto' token
            const std::string label = look_ahead.val + "." + current_func;

            // save in label table that label is used
            auto it = label_table.find(label);
            if (it == label_table.end()) {
                label_table[label] = Label_info{
                    .defined = false,
                    .used = true,
                    .loc = current.location
                };
            // already in table, just set 'used'
            } else {
                it->second.used = true;
            }

            expect(Token_type::Identifier, Error_kind::Missing_label, "after goto");
            expect(Token_type::Semicolon, Error_kind::Missing_semicolon, "at end of goto statement");

            const auto label_view = string_table->intern(label);
            return arena->allocate<ast::Goto>(ast::Goto(label_view, loc));
        }

        // labeled statement
        case Token_type::Identifier: {
            // might be a labeled statement
            if (lexer.peek(0).type == Token_type::Colon) {
                scan(); // label
                const auto label = current.val + "." + current_func;

                // add label to label table and check if its duplicate
                if (const auto it = label_table.find(label); it == label_table.end()) {
                    label_table[label] = Label_info{
                        .defined = true,
                        .used = false,
                        .loc = current.location
                    };
                } else {
                    // already in table
                    // might be from goto
                    if (not it->second.defined) {
                        assert(it->second.used && "in labeled statement check");
                        it->second.defined = true;
                    // already defined
                    } else {
                        std::string msg = "with name " + label;
                        diag.report_issue(Severity::Error, current, Error_kind::Duplicate_label, msg);
                        diag.report_issue(Severity::Note, it->second.loc, Error_kind::First_defined_here, "");
                    }
                }

                scan(); // colon

                auto stmt = parse_statement();
                const auto label_view = string_table->intern(label);
                return arena->allocate<ast::Labeled>(label_view, stmt, loc);
            }
            // if not a labeled statement, it's an expression statement, so just fallthrough to default case
        }

        // assume everything else to be an expression statement
        [[fallthrough]];
        default: {
            auto expr = parse_expression(0);
            if (expr->kind == ast::Expr::Kind::Error) return arena->allocate<ast::Error_stmt>(loc);
            if (la_type != Token_type::Semicolon) {
                // check if it's just a stray token
                if (lexer.peek(0).type == Token_type::Semicolon) {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in expression statement");
                    scan(); scan();
                // not just a stray token -> enter recovery mode
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_semicolon, "at end of expression statement");
                    return arena->allocate<ast::Error_stmt>(loc);
                }
            } else scan();
            return arena->allocate<ast::Expr_stmt>(expr, loc);
        }
    }
}
