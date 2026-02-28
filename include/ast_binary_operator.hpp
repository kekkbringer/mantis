//
// Created by dominik on 7/8/25.
//

#ifndef BINARY_OPERATOR_HPP
#define BINARY_OPERATOR_HPP

namespace ast {
    /**
     * Enum class that holds all possible types of general binary operations in the abstract syntax tree.
     */
    enum class Binary_operator {
    #define X(name) name,
    #include "def_files/ast_binary_operator.def"
    #undef X
    };

    /**
     * Returns the name of the binary operator as a string, useful for debugging and printing the AST.
     * @param t binary operator to be translated
     * @return name of the binary operator as string
     */
    inline const char* to_string(const Binary_operator t) {
        switch (t) {
    #define X(name) case ast::Binary_operator::name: return #name;
    #include "def_files/ast_binary_operator.def"
    #undef X
            default: return "<unknown>";
        }
    }
}

#endif //BINARY_OPERATOR_HPP
