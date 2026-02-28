//
// Created by dominik on 7/3/25.
//

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "token_type.hpp"
#include "source_location.hpp"

#include <string>
#include <utility>

#include "ast_binary_operator.hpp"

/**
 * @class Token
 * @brief Token class, consisting of the type of token, its line and column number and, optionally, its value if it's a
 * constant or identifier.
 */
class Token {
public:
    Token_type type; ///< type of token
    Source_location location; ///< location of the token
    std::string val; ///< token value, in case of identifier or constant

    /**
     * Default constructor initializes 'type' to 'None'
     */
    Token() : type(Token_type::None) {}

    /**
     * Constructor for tokens that do not keep a 'value'.
     * @param [in] tt Type of the token
     * @param [in] l Location of the token within the source code
     */
    Token(const Token_type tt, const Source_location& l) : type(tt), location(l) {}

    /**
     * Constructor for tokens like identifiers and constants, that keep a 'value'
     * @param [in] tt Type of the token
     * @param [in] l Location of the token within the source code
     * @param [in] v Value of the token
     */
    Token(const Token_type tt, const Source_location& l, std::string v) :
        type(tt), location(l), val(std::move(v)) {}

    /**
     * Equals operator compares all member variables
     * @param [in] other Token to test equality with
     * @return Returns true if all member variables are equal
     */
    bool operator==(const Token &other) const {
        return type == other.type && val == other.val
            && location.line == other.location.line && location.column == other.location.column
            && location.file_name == other.location.file_name && location.offset == other.location.offset;
    }

    /**
     * Specifier are: 'int', 'static' and 'extern'
     * @return 'true' if the token is a specifier, 'false' otherwise
     */
    [[nodiscard]] bool is_specifier() const {
        return type == Token_type::int_ || type == Token_type::static_ || type == Token_type::extern_;
    }

    /**
     * Unary operators are '-' and '~'
     * @return 'true' if the token is a unary operator, 'false' otherwise
     */
    [[nodiscard]] bool is_unary() const {
        return type == Token_type::Minus || type == Token_type::Complement || type == Token_type::Not;
    }

    /**
     * Binary operators are '*', '/', '%', '+', '-', ..., also '?'
     * @return 'true' if the token is a binary operator, 'false' otherwise
     */
    [[nodiscard]] bool is_binary() const {
        return type == Token_type::Plus       || type == Token_type::Minus          || type == Token_type::Times
            || type == Token_type::Slash      || type == Token_type::Remainder
            || type == Token_type::Bit_and    || type == Token_type::Bit_or         || type == Token_type::Bit_xor
            || type == Token_type::Shift_left || type == Token_type::Shift_right
            || type == Token_type::Equal      || type == Token_type::Not_equal
            || type == Token_type::Less       || type == Token_type::Less_equal
            || type == Token_type::Greater    || type == Token_type::Greater_equal
            || type == Token_type::And        || type == Token_type::Or
            || type == Token_type::Assign
            || type == Token_type::Compound_add || type == Token_type::Compound_subtract
            || type == Token_type::Compound_multiply || type == Token_type::Compound_divide || type == Token_type::Compound_remainder
            || type == Token_type::Compound_and || type == Token_type::Compound_or || type == Token_type::Compound_xor
            || type == Token_type::Compound_shiftl || type == Token_type::Compound_shiftr
            || type == Token_type::Question_mark;
    }

    [[nodiscard]] bool is_compound() const {
        return type == Token_type::Compound_add || type == Token_type::Compound_subtract
            || type == Token_type::Compound_multiply || type == Token_type::Compound_divide || type == Token_type::Compound_remainder
            || type == Token_type::Compound_and || type == Token_type::Compound_or || type == Token_type::Compound_xor
            || type == Token_type::Compound_shiftl || type == Token_type::Compound_shiftr;
    }
};

inline std::ostream& operator<<(std::ostream &os, const Token &t) {
    os << "Token  type=" << to_string(t.type) << "  line=" << t.location.line << "  col=" << t.location.column
       << "  offset=" << t.location.offset;
    if (!t.val.empty()) os << " -val: " << t.val;
    return os;
}

#endif //TOKEN_HPP
