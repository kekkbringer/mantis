//
// Created by dominik on 7/8/25.
//

#ifndef TAC_BINARY_OPERATOR_HPP
#define TAC_BINARY_OPERATOR_HPP

namespace tac {
    /**
     * Enum class that holds all possible types of general binary operations in the TAC tree.
     */
    enum class Binary_operator {
    #define X(name) name,
    #include "def_files/tac_binary_operator.def"
    #undef X
    };

    /**
     * Compile time reflexion that returns the name of the binary operation as a string.
     * @param t binary operator to be translated
     * @return name of the binary operator as string
     */
    inline const char* to_string(const Binary_operator t) {
        switch (t) {
    #define X(name) case Binary_operator::name: return #name;
    #include "def_files/tac_binary_operator.def"
    #undef X
            default: return "<unknown>";
        }
    }
}

#endif //TAC_BINARY_OPERATOR_HPP
