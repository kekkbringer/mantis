//
// Created by dominik on 2/13/26.
//

#include "code_emitter.hpp"

void Code_emitter::emit_static_variable(const assem::Static_variable& svar) const {
    if (svar.global) os << "\t.globl " << svar.name << "\n";

    // non-zero init
    if (svar.init != 0) {
        os << "\t.data\n";
        os << "\t.align 4\n";
        os << svar.name << ":\n";
        os << "\t.long " << svar.init << "\n";

    // init to zero -> BSS
    } else  {
        os << "\t.bss\n";
        os << "\t.align 4\n";
        os << svar.name << ":\n";
        os << "\t.zero 4\n";
    }
}
