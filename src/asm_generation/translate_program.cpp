//
// Created by dominik on 7/11/25.
//

#include <cassert>

#include "asm_generator.hpp"

/**
 * This function builds the assembler program tree from a TAC program tree by traversing the TAC top level constructs.
 * @param prog assembler program tree
 */
void Asm_generator::translate_program(assem::Program& prog) {
    for (const auto& tac_tl: tac_prog->top_level()) {
        if (tac_tl.kind == tac::Top_level::Kind::Function) {
            const auto asm_func = translate_function(tac_tl.func);
            prog.emplace_back(asm_func);

        } else if (tac_tl.kind == tac::Top_level::Kind::Static_variable) {
            const auto* stat_var = tac_tl.stat_var;
            prog.emplace_back(assem::Static_variable(stat_var->name.data(),
                                                     stat_var->global,
                                                     stat_var->init));

        } else {
            assert(false && "internal error in translate_top_level");
            std::unreachable();
        }
    }
}