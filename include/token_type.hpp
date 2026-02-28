//
// Created by dominik on 7/7/25.
//

#ifndef TOKEN_TYPE_HPP
#define TOKEN_TYPE_HPP

/**
* @enum Token_type
* @brief Enum class to track the type of token.
*
* The enum class consists of several categories of token types. Internal tokens like 'None' and 'EOF_', token classes
* like 'Identifier' and different types of constants, all valid operators and special characters like parentheses and
* braces and finally all keyword.
*/
enum class Token_type {
#define X(name) name,
#include "def_files/token_type.def"
#undef X
};

/**
 * Compile time reflexion that returns the name of the token type as a string.
 * @param t token type to be translated
 * @return name of the token type as string
 */
inline const char* to_string(const Token_type t) {
    switch (t) {
#define X(name) case Token_type::name: return #name;
#include "def_files/token_type.def"
#undef X
        default: return "<unknown>";
    }
}

#endif //TOKEN_TYPE_HPP
