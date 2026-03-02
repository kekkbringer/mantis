//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"
#include "util.hpp"

#include <cassert>
#include <vector>

tac::Function* Tac_generator::translate_function_decl(const ast::Func_decl* fdecl) {
    const auto name = fdecl->name;

    // fill function body by translating each block item on its own
    std::vector<tac::Inst*> body;
    if (fdecl->body != nullptr)
        for (const auto bitem : fdecl->body->item_span()) translate_block_item(&bitem, body);

    // add a return 0 to the end of each function
    // this is done to conform to the C standard when the main function has no return statement
    // other non-void functions not having a return statement is undefined behaviour, so this approach is still
    // standard conform in such cases
    auto ret0 = arena->allocate<tac::Return>(tac::Value::make_int(0));
    body.emplace_back(ret0);

    std::vector<std::string_view> tac_params;
    for (const auto& param: fdecl->parameters()) {
        tac_params.emplace_back(param->name);
    }

    // lookup symbol of function declaration
    const Symbol* sym = file_scope->find_sym(name.data());
    assert(sym != nullptr);
    const bool global = (sym->linkage == Symbol::Linkage::External);

    // copy to arena
    auto*  param_ptr = arena->allocate_array<std::string_view>(tac_params.size());
    auto** insts_ptr = arena->allocate_array<tac::Inst*>(body.size());
    std::ranges::copy(tac_params, param_ptr);
    std::ranges::copy(body, insts_ptr);
    const auto func = arena->allocate<tac::Function>(fdecl->name, global,
                                                    param_ptr, tac_params.size(), insts_ptr, body.size());

    return func;
}

std::optional<tac::Top_level> Tac_generator::translate_declaration(const ast::Decl* decl) {
    if (decl->kind == ast::Decl::Kind::Func) {
        const auto func_decl = cast<const ast::Func_decl>(decl);
        if (func_decl->body != nullptr) {
            return tac::Top_level(translate_function_decl(func_decl));
        }
    }
    return {};
}