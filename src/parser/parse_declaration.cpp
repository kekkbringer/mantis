//
// Created by dominik on 7/5/25.
//

#include "parser.hpp"

/**
 * This function is called after a catastrophic error, usually a non-trivial syntax error, is encountered to
 * synchronize the incoming token stream to a safe reentry point. The save point should represent a place where parsing
 * a declaration can be started again.
 */
void Parser::synchronize_declaration() {
    scan(); // consume bad token, prevent infinite loops

    while (la_type != Token_type::EOF_) {
        // reentry after a semicolon should always be safe
        if (current.type == Token_type::Semicolon) {
            in_error_recovery = false;
            return;
        }

        // always resume with a specifier
        if (look_ahead.is_specifier()) {
            in_error_recovery = false;
            return;
        }

        // check for initial tokens of declarations
        switch (current.type) {
        case Token_type::int_:
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
 * This is the wrapper routine for the actual declaration parsing function. This wrapper handles recovery from error
 * mode after a catastrophic error was encountered by resynchronizing the incoming token stream to a safe reentry point.
 * The parsing routine indicates a catastrophic error by returning an ErrorDecl. In this case we try to recover and
 * return a pointer to an ErrorDecl node ourselves.
 * @return
 */
ast::Declaration_ptr Parser::parse_declaration() {
    // if we are already in error recovery mode -> synchronize
    if (in_error_recovery) {
        synchronize_declaration();
        return ast::ErrorDecl_ptr();
    }

    // call actual parsing function
    auto decl_ptr = parse_declaration_inner();

    // check if catastrophic error was encountered
    if (std::holds_alternative<ast::ErrorDecl_ptr>(decl_ptr)) {
        synchronize_declaration();
        return ast::ErrorDecl_ptr();
    }

    return decl_ptr;
}

/**
 * Function to parse a declaration with the following syntax in EBNF:
 * <declaration> ::= <variable-declaration> | <function-declaration>
 * <variable-declaration> ::= { <specifier> }+ <identifier> [ "=" <exp> ] ";"
 * <function-declaration> ::= { <specifier> }+ <identifier> "(" <param-list> ")" ( <block> | ";" )
 * @return unique_ptr to a declaration node
 */
ast::Declaration_ptr Parser::parse_declaration_inner() {

    // <declaration> ::= <variable-declaration> | <function-declaration>
    // <variable-declaration> ::= { <specifier> }+ <identifier> [ "=" <exp> ] ";"
    // <function-declaration> ::= { <specifier> }+ <identifier> "(" <param-list> ")" ( <block> | ";" )

    Source_location loc;

    // there must be at minimum one specifier, currently 'int' must always be present
    bool is_int = false;
    auto sc = ast::Storage_class::None;
    while (look_ahead.is_specifier()) {
        switch (la_type) {
            // data types
            case Token_type::int_: {
                if (is_int) diag.report_issue(Severity::Error, look_ahead, Error_kind::Duplicate_typename, ", was already declared 'int'");
                is_int = true;
                scan();
                break;
            }

            // storage classifier
            case Token_type::static_: {
                if (sc == ast::Storage_class::None) {
                    sc = ast::Storage_class::Static;
                } else {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Conflicting_storage_class, "");
                }
                scan();
                break;
            }

            case Token_type::extern_: {
                if (sc == ast::Storage_class::None) {
                    sc = ast::Storage_class::Extern;
                } else {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Conflicting_storage_class, "");
                }
                scan();
                break;
            }

            default:
                diag.report_issue(Severity::Error, look_ahead, Error_kind::Unknown, " storage class specifier");
        }
    }

    // all specifier have been read, check if data type (so far only 'int') is present
    if (not is_int) {
        // TODO: this might better be handled as a catastrophic error
        diag.report_issue(Severity::Error, look_ahead, Error_kind::Typeless_declaration);
    }

    // identifier must be next
    std::string name;
    if (la_type == Token_type::Identifier) {
        scan();
        name = current.val;
        loc = current.location;
    } else {
        if (lexer.peek(0).type == Token_type::Identifier) {
            diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "before identifier");
            scan(); scan();
        } else {
            report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_identifier, "after specifier in declaration");
            return ast::ErrorDecl_ptr();
        }
    }


    // <variable-declaration> ::= { <specifier> }+ <identifier> [ "=" <exp> ] ";"
    // <function-declaration> ::= { <specifier> }+ <identifier> "(" <param-list> ")" ( <block> | ";" )

    // function declaration
    if (la_type == Token_type::Open_parenthesis) {
        scan();

        // parse parameter list, which enters a new scope
        enter_new_scope();
        std::vector<std::shared_ptr<Type>> param_types;
        auto param_list = parse_param_list(param_types);
        if (la_type != Token_type::Close_parenthesis) {
            // just a stray token?
            if (lexer.peek(0).type == Token_type::Close_parenthesis) {
                diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in function parameter list");
                scan(); scan();
            } else {
                report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_closing_parenthesis, "after parameter list in function declaration");
                return ast::ErrorDecl_ptr();
            }
        } else scan();

        auto func_decl = std::make_unique<ast::Function_declaration>(name, param_list, sc);
        func_decl->loc = loc;

        // construct symbol
        Symbol sym;
        sym.name = func_decl->name;
        sym.decl = func_decl.get();
        sym.kind = Symbol::Kind::Func;
        sym.linkage = Symbol::Linkage::External;
        sym.type = type_pool.get_function(type_pool.type_int, param_types);

        // block scope 'static' function is not valid
        if (current_scope->parent->parent != nullptr) {
            if (sc == ast::Storage_class::Static) {
                std::string msg = "of function " + sym.name;
                diag.report_issue(Severity::Error, loc, Error_kind::Invalid_storage_class, msg);
            }
        }

        // check if function is already in symbol table
        const Symbol* lookup_sym = current_scope->find_global(name);
        if (lookup_sym != nullptr) {
            if (lookup_sym->kind == Symbol::Kind::Func) {
                // check for mismatched types
                if (*lookup_sym->type != *sym.type) {
                    std::string msg = "with name " + sym.name;
                    diag.report_issue(Severity::Error, loc, Error_kind::Incompatible_function_redeclaration, msg);
                    diag.report_issue(Severity::Note, get_location(lookup_sym), Error_kind::First_defined_here, "");
                }

                // check if a static function declaration follows a non-static declaration
                if (lookup_sym->linkage == Symbol::Linkage::External and sc == ast::Storage_class::Static) {
                    std::string msg = "with name " + sym.name + ". First defined non-static, now static.";
                    diag.report_issue(Severity::Error, loc, Error_kind::Incompatible_function_redeclaration, msg);
                    diag.report_issue(Severity::Note, get_location(lookup_sym), Error_kind::First_defined_here, "");
                }

                // carry over global status from previous declaration (function redeclarations behave as though they
                // had the 'extern' keyword in this sense)
                sym.linkage = lookup_sym->linkage;

            // was already defined as non-functionality
            // previous declaration will not be shadows if it had linkage
            } else if (lookup_sym->linkage == Symbol::Linkage::Internal or lookup_sym->linkage == Symbol::Linkage::External) {
                diag.report_issue(Severity::Error, loc, Error_kind::Redeclaration_different_type, "found function");
                diag.report_issue(Severity::Note, get_location(lookup_sym), Error_kind::First_defined_here, "");
            }
        }

        // check if variable is being redeclared
        const auto upper_sym = current_scope->parent->lookup(name);
        if (current_scope->parent->symbols.contains(name)) {
            if (upper_sym->kind==Symbol::Kind::Var) {
                std::string msg = ", variable " + name + " is redeclared as function";
                diag.report_issue(Severity::Error, loc, Error_kind::Illegal_declaration, msg);
                diag.report_issue(Severity::Note, get_location(current_scope->parent->lookup(name)), Error_kind::First_defined_here, "");
            }
        }

        // declaration, no body
        if (la_type == Token_type::Semicolon) {
            scan(); // ';'

            // declare function (in outer scope, outside of its parameter decls)
            leave_scope();
            current_scope->declare(sym);

            return func_decl;

        //definition, body is present, if and only if we encounter a open brace
        } else if (la_type == Token_type::Open_brace) {
            // check for function redefinition
            if (upper_sym != nullptr and std::get<ast::Function_declaration*>(upper_sym->decl)->body != nullptr) {
                std::string msg = "with name " + name;
                diag.report_issue(Severity::Error, current, Error_kind::Redefinition_of_function, msg);
                diag.report_issue(Severity::Note, get_location(upper_sym), Error_kind::First_defined_here, "");
            }

            // test for nested function definitions and remember current function
            if (not current_func.empty()) {
                std::string msg = "of function " + current_func + " within function " + name;
                diag.report_issue(Severity::Error, loc, Error_kind::Nested_function_definitions, msg);
            }
            current_func = name;

            // in new function, clear function scope label table
            label_table.clear();

            current_scope->parent->declare(sym);

            // parse block without entering a new scope, the scope begins with the parameter list
            auto body = parse_block(false);
            func_decl->body = std::make_unique<ast::Block>(std::move(body));
            func_decl->loc = loc;

            leave_scope();

            // check label table
            for (const auto& label_info: label_table) {
                if (label_info.second.used and not label_info.second.defined) {
                    std::string msg = "in function " + name + " with name " + label_info.first;
                    diag.report_issue(Severity::Error, label_info.second.loc, Error_kind::Undefined_label, msg);
                }
                if (not label_info.second.used) {
                    std::string msg = "in function " + name;
                    diag.report_issue(Severity::Warning, label_info.second.loc, Error_kind::Unused_label, msg);
                }
            }

            current_func = "";
            return func_decl;

        // no semicolon (declaration) and no open brace (definition)
        } else {
            report_syntax_error(Severity::Error, look_ahead, Error_kind::Invalid_function_declaration);
        }


    // variable
    } else if (la_type == Token_type::Semicolon or la_type == Token_type::Assign) {
        // prepare return
        auto var_decl_ptr = std::make_unique<ast::Variable_declaration>(name, sc);

        // create general part of the symbol
        Symbol sym;
        sym.name = var_decl_ptr->name;
        sym.decl = var_decl_ptr.get();
        sym.kind = Symbol::Kind::Var;
        sym.type = type_pool.type_int;

        const bool has_init = la_type == Token_type::Assign;
        scan();

        // check full/true init of variable, might be: None, Initial or Tentative
        if (has_init) sym.init = Symbol::Init::Initial;
        else sym.init = (sc == ast::Storage_class::Extern ? Symbol::Init::None : Symbol::Init::Tentative);

        // distinguish between file scope or block scope
        const bool at_file_scope = current_scope->parent == nullptr;

        // deal with linkage and storage duration first
        if (at_file_scope) {
            sym.unique_name = sym.name;
            sym.storage_duration = Symbol::Storage_duration::Static;
            sym.linkage = (sc == ast::Storage_class::Static) ? Symbol::Linkage::Internal : Symbol::Linkage::External;

            // warm about file scope 'extern' eith init
            if (has_init and sym.linkage == Symbol::Linkage::External)
                diag.report_issue(Severity::Warning, loc, Error_kind::Extern_with_init, "");

        // block scope linkage and storage duration
        } else {
            // 'static' block scope
            if (sc == ast::Storage_class::Static) {
                sym.linkage = Symbol::Linkage::None;
                sym.storage_duration = Symbol::Storage_duration::Static;
                if (not has_init) {
                    sym.init = Symbol::Init::Initial;
                    var_decl_ptr->init = std::make_unique<ast::Constant>(0);
                }
            // 'extern' block scope
            } else if (sc == ast::Storage_class::Extern) {
                sym.unique_name = sym.name; // resolves to file scope variable
                sym.linkage = Symbol::Linkage::External;
                sym.storage_duration = Symbol::Storage_duration::Static;
                if (has_init) diag.report_issue(Severity::Error, loc, Error_kind::Extern_with_init, "");
            // automatic storage duration block scope variable
            } else {
                sym.linkage = Symbol::Linkage::None;
                sym.storage_duration = Symbol::Storage_duration::Automatic;
            }
        }

        // check for redeclaration and insert?
        if (at_file_scope) {
            auto it = current_scope->symbols.find(sym.name);
            if (it != current_scope->symbols.end()) {
                Symbol& prev = it->second;
                // redeclaration must have the same type
                if (prev.type != sym.type) {
                    diag.report_issue(Severity::Error, loc, Error_kind::Redeclaration_different_type, "");
                    diag.report_issue(Severity::Note, get_location(&prev), Error_kind::First_defined_here, "");
                }
                // redeclaration must have the same linkage
                // or this one must have the 'extern' keyword, in which case the previous linkage is adopted
                if (prev.linkage != sym.linkage) {
                    if (sc == ast::Storage_class::Extern) sym.linkage = prev.linkage;
                    else {
                        diag.report_issue(Severity::Error, loc, Error_kind::Conflicting_linkage);
                        diag.report_issue(Severity::Note, get_location(&prev), Error_kind::First_defined_here, "");
                    }
                }
                // may not have a second initializer
                if (prev.init == Symbol::Init::Initial) {
                    if (sym.init == Symbol::Init::Initial) {
                        diag.report_issue(Severity::Error, loc, Error_kind::Conflicting_file_scope_definition);
                        diag.report_issue(Severity::Note, get_location(&prev), Error_kind::First_defined_here, "");
                    }
                    // prev had the init, keep it, don't overwrite, just update linkage if needed
                    it->second.linkage = sym.linkage;
                } else {
                    // save to overwrite, prev had no real initializer
                    if (prev.init == Symbol::Init::Tentative and sym.init != Symbol::Init::Initial) {
                        sym.init = Symbol::Init::Tentative;
                    }
                    it->second = sym;
                }
            // not known from current scope -> declare
            } else {
                current_scope->declare(sym);
            }

            // an 'extern' block scope variable might have brought something into scope
            if (sym.linkage == Symbol::Linkage::Internal) {
                const Symbol* extern_sym = current_scope->get_file_scope()->any_external_linkage(name);
                if (extern_sym != nullptr) {
                    diag.report_issue(Severity::Error, loc, Error_kind::Conflicting_linkage, "");
                    diag.report_issue(Severity::Note, get_location(extern_sym), Error_kind::First_defined_here, "");
                }
            }

        // block scope
        } else {
            // block scope 'extern'
            if (sc == ast::Storage_class::Extern) {
                // check if name is already known in the current scope with no linkage
                const auto it = current_scope->symbols.find(sym.name);
                if (it != current_scope->symbols.end()) {
                    if (it->second.linkage == Symbol::Linkage::None) {
                        diag.report_issue(Severity::Error, loc, Error_kind::Conflicting_linkage);
                        diag.report_issue(Severity::Note, get_location(&it->second), Error_kind::First_defined_here, "");
                    }
                }

                // find the file-scope symbol, no need to create a new one
                const Symbol* file_sym = current_scope->lookup_file_scope(sym.name);
                if (file_sym != nullptr) {
                    // local redeclaration must have the same type
                    if (file_sym->type != sym.type) {
                        diag.report_issue(Severity::Error, loc, Error_kind::Redeclaration_different_type, "");
                        diag.report_issue(Severity::Note, get_location(file_sym), Error_kind::First_defined_here, "");
                    }
                    // file scope declaration must have hab external linkage
                    if (file_sym->linkage == Symbol::Linkage::None) {
                        diag.report_issue(Severity::Error, loc, Error_kind::Conflicting_linkage);
                        diag.report_issue(Severity::Note, get_location(file_sym), Error_kind::First_defined_here, "");
                    }
                }
                // declare in current scope to 'bring back into scope'
                current_scope->declare(sym);

            // block scope 'static'
            } else if (sc == ast::Storage_class::Static) {
                // check for redeclaration
                if (current_scope->symbols.contains(sym.name)) {
                    const std::string msg = "with name " + sym.name;
                    diag.report_issue(Severity::Error, current, Error_kind::Duplicate_variable_decl, msg);
                    diag.report_issue(Severity::Note, get_location(current_scope->lookup(name)),
                                                                            Error_kind::First_defined_here, "");
                }
                // declare while mangling name
                current_scope->declare(sym, mangle_counter);

                // also add mangled name to file scope so it is found later when replacing pseudo register
                //current_scope->get_file_scope()->symbols[sym.unique_name] = sym;

            // block scope automatic storage duration variable
            } else {
                // declare variable in current scope (automatically check for duplicate declaration)
                if (not current_scope->declare(sym, mangle_counter)) {
                    const std::string msg = "with name " + sym.name;
                    diag.report_issue(Severity::Error, current, Error_kind::Duplicate_variable_decl, msg);
                    diag.report_issue(Severity::Note, get_location(current_scope->lookup(name)),
                                                        Error_kind::First_defined_here, "");
                }
            }
        }


        // if present, parse init expression
        // important to have added the variable to the symbol table first, as it is in scope in its own init
        // e.g. int a = a = 5;    or     int a = sizeof(a);
        if (has_init) {
            auto expr = parse_expression(0);
            // check for error in expression parsing
            if (std::holds_alternative<ast::ErrorExpr_ptr>(expr)) return ast::ErrorDecl_ptr();
            if (la_type != Token_type::Semicolon) {
                // check for stray token
                if (lexer.peek(0).type == Token_type::Semicolon) {
                    diag.report_issue(Severity::Error, look_ahead, Error_kind::Stray_token, "in variable initialization");
                    scan(); scan();
                } else {
                    report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_semicolon,  "after variable initialization");
                    return ast::ErrorDecl_ptr();
                }
            } else scan();
            var_decl_ptr->init = std::move(expr);

            // if file scope variable with initialization, check if init is a constant
            // (TODO: this should really be a constexpr)
            if (current_scope->parent == nullptr) {
                if (not std::holds_alternative<ast::Constant_ptr>(var_decl_ptr->init))
                    diag.report_issue(Severity::Error, loc, Error_kind::Non_const_init, "of file scope variable");

            // a block scope 'static' variable also needs to have a const init
            // (TODO: this should really be a constexpr)
            } else if (sc == ast::Storage_class::Static) {
                if (not std::holds_alternative<ast::Constant_ptr>(var_decl_ptr->init))
                    diag.report_issue(Severity::Error, loc, Error_kind::Non_const_init, "of 'static' block scope variable");
            }
        }

        // use (potentially) mangled name in AST and return
        var_decl_ptr->name = sym.unique_name;
        var_decl_ptr->loc = loc;
        return var_decl_ptr;

    } else {
        report_syntax_error(Severity::Error, look_ahead, Error_kind::Missing_declaration, "");
    }

    auto func_decl = std::make_unique<ast::Function_declaration>(name, sc);
    func_decl->loc = loc;
    return func_decl;
}
