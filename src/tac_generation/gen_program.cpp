//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"

tac::Program* Tac_generator::gen_program() {
    std::vector<tac::Top_level> tmp_top_level;

    // traverse AST
    for (const auto& decl: prog->declarations()) {
        if (decl != nullptr)
        if (auto tl = translate_declaration(decl)) tmp_top_level.emplace_back(*tl);
    }

    // traverse symbol table and emit variables with static storage duration
    traverse_scope(file_scope, tmp_top_level);

    // copy into arena
    auto* tl_ptr = arena->allocate_array<tac::Top_level>(tmp_top_level.size());
    std::ranges::copy(tmp_top_level, tl_ptr);
    const auto tac_prog = arena->allocate<tac::Program>(tl_ptr, tmp_top_level.size());

    return tac_prog;
};