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
    for (const auto& tac_tl: tac_prog) {
        //translate_top_level(prog, tac_tl);
        std::visit([&]<class T0>(T0&& arg){
            using T = std::decay_t<T0>;

            // function definition
            if constexpr (std::is_same_v<T, tac::Function_ptr>) {
                if (arg == nullptr) return;
                prog.emplace_back(translate_function(arg));

            // static variable
            } else if constexpr (std::is_same_v<T, tac::Static_variable_ptr>) {
                if (arg == nullptr) return;
                // simply place a static variable
                prog.emplace_back(assem::Static_variable(arg->name, arg->global, arg->init));

            // unexpected/impossible construct
            } else {
                assert(false && "internal error in translate_top_level");
            }
        }, tac_tl);
    }
}