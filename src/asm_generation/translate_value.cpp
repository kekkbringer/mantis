//
// Created by dominik on 7/11/25.
//

#include "asm_generator.hpp"

#include <cassert>

/**
 * Translates a TAC value to the corresponding assembly operand, possible TAC values are constants and variables. The
 * function will return the new assembly operand, which will either be an immediate value in the case of a constant or
 * a pseudo register in the case of a variable.
 * @param vp TAC value to be translated
 * @return assembly operand
 */
assem::Operand Asm_generator::translate_value(const tac::Value_ptr& vp) {
    assem::Operand operand;

    std::visit([&]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;

        // constant -> immediate value
        if constexpr (std::is_same_v<T, tac::Constant>) {
            operand = assem::Immediate_value(arg.val);

        // variable -> pseudo register
        } else if constexpr (std::is_same_v<T, tac::Variable>) {
            operand = assem::Pseudo(arg.name);

        } else if constexpr (std::is_same_v<T, std::monostate>) {
        // unexpected/impossible branch
        } else {
            assert(false && "internal error in translate_value");
        }
    }, *vp);

    return operand;
}