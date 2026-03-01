//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"
#include "util.hpp"

#include <cassert>
#include <variant>
#include <vector>

tac::Function Tac_generator::translate_function_decl(const ast::Func_decl* fdecl) {
    const auto name = fdecl->name;

    // fill function body by translating each block item on its own
    std::vector<tac::Instruction_ptr> body;
    if (fdecl->body != nullptr)
        for (const auto bitem : fdecl->body->item_span()) translate_block_item(&bitem, body);

    // add a return 0 to the end of each function
    // this is done to conform to the C standard when the main function has no return statement
    // other non-void functions not having a return statement is undefined behaviour, so this approach is still
    // standard conform in such cases
    body.emplace_back(std::make_unique<tac::Return>(tac::Constant(0)));

    std::vector<std::string> tac_params;
    for (const auto& param: fdecl->parameters()) {
        std::cout << "HALLO: " << param->name.data() << std::endl;
        tac_params.emplace_back(param->name);
    }

    // lookup symbol of function declaration
    const Symbol* sym = file_scope->find_sym(name.data());
    assert(sym != nullptr);
    const bool global = (sym->linkage == Symbol::Linkage::External);

    return tac::Function(std::string(name), global, std::move(tac_params), std::move(body));
}

tac::Top_level_ptr Tac_generator::translate_declaration(const ast::Decl* decl) {
    tac::Top_level_ptr tlp;

    if (decl->kind == ast::Decl::Kind::Func) {
        auto func_decl = cast<const ast::Func_decl>(decl);
        if (func_decl->body != nullptr) tlp = std::make_unique<tac::Function>(translate_function_decl(func_decl));
    }

    return tlp;
}