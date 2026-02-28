//
// Created by dominik on 7/8/25.
//

#ifndef UNARY_OPERATOR_HPP
#define UNARY_OPERATOR_HPP

namespace ast {
    /**
     * Enum class that holds all possible types of general unary operations in the abstract syntax tree.
     */
    enum class Unary_operator {
    #define X(name) name,
    #include "def_files/unary_operator.def"
    #undef X
    };

    /**
     * Returns the name of the unary operator as a string, useful for debugging and printing the AST.
     * @param t unary operator to be translated
     * @return name of the unary operator as string
     */
    inline const char* to_string(const Unary_operator t) {
        switch (t) {
    #define X(name) case ast::Unary_operator::name: return #name;
    #include "def_files/unary_operator.def"
    #undef X
            default: return "<unknown>";
        }
    }
}
#endif //UNARY_OPERATOR_HPP
