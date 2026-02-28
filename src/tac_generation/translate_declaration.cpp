//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"

#include <cassert>
#include <variant>
#include <vector>

tac::Function Tac_generator::translate_function_decl(const ast::Function_declaration_ptr& fdecl) {
    const std::string name = fdecl->name;


    // fill function body by translating each block item on its own
    std::vector<tac::Instruction_ptr> body;
    if (fdecl->body != nullptr)
    for (const auto& bitem : *fdecl->body) {
        translate_block_item(bitem, body);
    }

    // add a return 0 to the end of each function
    // this is done to conform to the C standard when the main function has no return statement
    // other non-void functions not having a return statement is undefined behaviour, so this approach is still
    // standard conform in such cases
    body.emplace_back(std::make_unique<tac::Return>(tac::Constant(0)));

    std::vector<std::string> tac_params;
    for (const auto& param: fdecl->params) {
        tac_params.emplace_back(param->name);
    }

    // lookup symbol of function declaration
    const Symbol* sym = file_scope->find_sym(name);
    assert(sym != nullptr);
    const bool global = (sym->linkage == Symbol::Linkage::External);

    return tac::Function(name, global, std::move(tac_params), std::move(body));
}

tac::Top_level_ptr Tac_generator::translate_declaration(const ast::Declaration_ptr& decl) {
    tac::Top_level_ptr tlp;
    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, ast::Function_declaration_ptr>) {
            if (arg->body != nullptr) tlp = std::make_unique<tac::Function>(translate_function_decl(arg));
        } else if constexpr (std::is_same_v<T, ast::Variable_declaration_ptr>) {

        } else {
            assert(false && "could not translate declaration");
        }
    }, decl);

    return tlp;
}