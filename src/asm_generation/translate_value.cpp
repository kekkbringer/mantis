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
assem::Operand Asm_generator::translate_value(const tac::Value& vp) {
    assem::Operand operand;

    if (vp.kind == tac::Value::Kind::Int_constant) {
        operand = assem::Immediate_value(vp.int_val);

    } else if (vp.kind == tac::Value::Kind::Variable) {
        operand = assem::Pseudo(vp.name.data());

    } else {
        assert(false && "internal error in translate_value");
        std::unreachable();
    }

    return operand;
}