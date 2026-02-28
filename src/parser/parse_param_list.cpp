//
// Created by dominik on 7/7/25.
//

#include "parser.hpp"

Source_location get_location(const Symbol* sym);

/**
 * Function to parse a parameter list with the following syntax in EBNF:
 * <param-list> ::= "" | "void" | "int" <identifier> { "," "int" <identifier> }
 * @return std::vector of parameter as strings
 */
std::vector<ast::Variable_declaration_ptr> Parser::parse_param_list(std::vector<std::shared_ptr<Type>>& type_list) {
    std::vector<ast::Variable_declaration_ptr> param_list;

    // only void
    if (la_type == Token_type::void_) { scan(); return param_list; }

    // list
    if (la_type == Token_type::int_) {
        scan();
        expect(Token_type::Identifier, Error_kind::Missing_identifier, "after 'int' in parameter list");

        auto name = current.val;
        auto var_decl_ptr = std::make_unique<ast::Variable_declaration>(name, ast::Storage_class::None);

        // construct symbol
        Symbol sym;
        sym.name = var_decl_ptr->name;
        sym.decl = var_decl_ptr.get();
        sym.kind = Symbol::Kind::Var;

        // declare variable in current scope (automatically check for duplicate declaration)
        if (not current_scope->declare(sym, mangle_counter)) {
            std::string msg = "with name " + sym.name;
            diag.report_issue(Severity::Error, current, Error_kind::Duplicate_variable_decl, msg);
            diag.report_issue(Severity::Note, get_location(current_scope->lookup(name)), Error_kind::First_defined_here, "");
        }

        // use mangled name in AST and return
        var_decl_ptr->name = sym.unique_name;
        var_decl_ptr->loc = current.location;

        param_list.push_back(std::move(var_decl_ptr));
        type_list.push_back(type_pool.type_int);


        // { "," "int" <identifier> }
        while (la_type == Token_type::Comma) {
            scan();
            expect(Token_type::int_, Error_kind::Missing_keyword, "int", "after ',' in parameter list");
            expect(Token_type::Identifier, Error_kind::Missing_identifier, "after 'int' in parameter list");

            auto name = current.val;
            auto var_decl_ptr = std::make_unique<ast::Variable_declaration>(name, ast::Storage_class::None);

            // construct symbol
            Symbol sym;
            sym.name = var_decl_ptr->name;
            sym.decl = var_decl_ptr.get();
            sym.kind = Symbol::Kind::Var;

            // declare variable in current scope (automatically check for duplicate declaration)
            if (not current_scope->declare(sym, mangle_counter)) {
                std::string msg = "with name " + sym.name;
                diag.report_issue(Severity::Error, current, Error_kind::Duplicate_variable_decl, msg);
                diag.report_issue(Severity::Note, get_location(current_scope->lookup(name)), Error_kind::First_defined_here, "");
            }

            // use mangled name in AST and return
            var_decl_ptr->name = sym.unique_name;
            var_decl_ptr->loc = current.location;

            param_list.push_back(std::move(var_decl_ptr));
            type_list.push_back(type_pool.type_int);
        }


    // error
    } else {
        // empty parameter list is now legal in mantis
        //error("unexpected token in parameter list");
    }

    return param_list;
}
