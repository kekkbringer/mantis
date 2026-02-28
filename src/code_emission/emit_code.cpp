//
// Created by dominik on 7/12/25.
//

#include "code_emitter.hpp"

/**
 * Main driver of code emission, will traverse the assembly program and write the corresponding source code to the
 * ostream specified in the emitter.
 */
void Code_emitter::emit_code() {
#ifdef DEBUG_MODE
    os << "# ASSEMBLER CODE EMISSION\n";
#endif

    for (const auto& top_level: prog) {
        emit_top_level(top_level);
    }

    // make stack non-executable
    os << "\t.section .note.GNU-stack,\"\",@progbits\n";

}
