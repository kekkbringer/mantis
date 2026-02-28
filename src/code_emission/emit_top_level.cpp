//
// Created by dominik on 7/12/25.
//

#include "code_emitter.hpp"

#include <cassert>

/**
 * Emits an assembly top level construct, either a function definition or a static variable.
 * @param tp top level construct to be emitted
 */
void Code_emitter::emit_top_level(const assem::Top_level& tp) {
    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;

        // function definition
        if constexpr (std::is_same_v<T, assem::Function>) {
            emit_function_definition(arg);

        // static variable
        } else if constexpr (std::is_same_v<T, assem::Static_variable>) {
            emit_static_variable(arg);

        } else {
            assert(false && "internal error in emit_top_level");
        }
    }, tp);
}
