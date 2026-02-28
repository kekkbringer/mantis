//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"

tac::Program Tac_generator::gen_program() {
    std::vector<tac::Top_level_ptr> tac_prog;

    // traverse AST
    for (const auto& decl: prog.declarations) {
        tac_prog.emplace_back(translate_declaration(decl));
    }

    // traverse symbol table and emit variables with static storage duration if ne
    traverse_scope(file_scope, tac_prog);

    return tac_prog;
};