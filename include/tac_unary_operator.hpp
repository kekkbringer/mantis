//
// Created by dominik on 7/8/25.
//

#ifndef TAC_UNARY_OPERATOR_HPP
#define TAC_UNARY_OPERATOR_HPP

namespace tac {
    /**
     * Enum class that holds all possible types of general unary operations in the TAC tree.
     */
    enum class Unary_operator {
    #define X(name) name,
    #include "def_files/unary_operator.def"
    #undef X
    };

    /**
     * Compile time reflexion that returns the name of the unary operation as a string.
     * @param t unary operator to be translated
     * @return name of the unary operator as string
     */
    inline const char* to_string(const tac::Unary_operator t) {
        switch (t) {
    #define X(name) case tac::Unary_operator::name: return #name;
    #include "def_files/unary_operator.def"
    #undef X
            default: return "<unknown>";
        }
    }
}
#endif //TAC_UNARY_OPERATOR_HPP
