//
// Created by dominik on 7/3/25.
//

#include "lexer.hpp"

#include <iostream>

// list of all keywords  as strings
constexpr std::string_view keywords[] = {
    "alignas", "alignof", "auto", "bool", "break", "case", "char",
    "const", "constexpr", "continue", "default", "do", "double",
    "else", "enum", "extern", "false", "float", "for", "goto", "if",
    "inline", "int", "long", "nullptr", "register", "restrict",
    "return", "short", "signed", "sizeof", "static",
    "static_assert", "struct", "switch", "thread_local", "true",
    "typedef", "typeof", "typeof_unequal", "union", "unsigned",
    "void", "volatile", "while",
    "_Alignas", "_Alignof", "_Atomic", "_BitInt", "_Bool",
    "_Complex", "_Decimal128", "_Decimal64", "_Decimal32",
    "_Generic", "_Imaginary", "_Noreturn", "_Static_assert",
    "_Thread_local"
};

// list of all keyword Token_types in the same order
constexpr Token_type keyword_types[] = {
    Token_type::alignas_, Token_type::alignof_, Token_type::auto_, Token_type::bool_, Token_type::break_,
    Token_type::case_, Token_type::char_, Token_type::const_, Token_type::constexpr_, Token_type::continue_,
    Token_type::default_, Token_type::do_, Token_type::double_, Token_type::else_, Token_type::enum_,
    Token_type::extern_, Token_type::false_, Token_type::float_, Token_type::for_, Token_type::goto_, Token_type::if_,
    Token_type::inline_, Token_type::int_, Token_type::long_, Token_type::nullptr_, Token_type::register_,
    Token_type::restrict_, Token_type::return_, Token_type::short_, Token_type::signed_, Token_type::sizeof_,
    Token_type::static_, Token_type::static_assert_, Token_type::struct_, Token_type::switch_,
    Token_type::thread_local_, Token_type::true_, Token_type::typedef_, Token_type::typeof_,
    Token_type::typeof_unequal_, Token_type::union_, Token_type::unsigned_, Token_type::void_, Token_type::volatile_,
    Token_type::while_, Token_type::Alignas_, Token_type::Alignof_, Token_type::Atomic_, Token_type::BitInt_,
    Token_type::Bool_, Token_type::Complex_, Token_type::Decimal128_, Token_type::Decimal64_, Token_type::Decimal32_,
    Token_type::Generic_, Token_type::Imaginary_, Token_type::Noreturn_, Token_type::Static_assert_,
    Token_type::Thread_local_,
};

/**
 * This function advances the 'ch' pointer to the next character
 *
 * This function advances the pointer 'ch' to the next character of the input, while keeping track of the line and
 * column number. It returns 1 if the end of the input is reached and 0 otherwise.
 * @return 1 if end of input is reached, 0 otherwise
 */
int Lexer::next_char() {
    if (++offset >= source.size()) {
        current_char = '\0';
        return 1; //eof
    }

    current_char = source[offset];

    if (current_char == '\n') {
        ++line;
        col = 0;
    } else {
        col++;
    }

    return 0;
}

/**
 * @brief peek at the nth token in the stream without advancing it.
 */
Token Lexer::peek(const size_t n) {
    // fill look ahead buffer until the requested token
    while (look_ahead_buffer.size() <= n) {
        Token t = read_token();
        look_ahead_buffer.push_back(t);
    }

    return look_ahead_buffer[n];
}


/**
 * This function returns the next token from the input.
 *
 * The function first checks for white spaces and other discardable characters.
 * @return Next token from input
 */
Token Lexer::next() {
    if (!look_ahead_buffer.empty()) {
        Token t = look_ahead_buffer.front();
        look_ahead_buffer.pop_front();
        return t;
    }

    return read_token();
}

/**
 * This function returns the next token from the input.
 *
 * The function first checks for white spaces and other discardable characters.
 * @return Next token from input
 */
Token Lexer::read_token() {
    // skip whitespaces
    while (offset < source.size() && std::isspace(current_char)) { next_char(); }

    if (offset >= source.size()) {
        return {Token_type::EOF_, Source_location(source_manager.get_file_name(), offset, line, col)};
    }

    // save location at beginning of token
    const Source_location loc(source_manager.get_file_name(), offset, line, col);
    Token token(Token_type::None, loc);
    bool error_given = false;

    switch (current_char) {
        // identifier of keyword
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k':
        case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v':
        case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K':
        case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
        case 'W': case 'X': case 'Y': case 'Z':
        case '_':
            read_name(token); break;

        // number
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            read_number(token); break;

        // single character tokens
        case ',': next_char(); token.type = Token_type::Comma; break;
        case ';': next_char(); token.type = Token_type::Semicolon; break;
        case '(': next_char(); token.type = Token_type::Open_parenthesis; break;
        case ')': next_char(); token.type = Token_type::Close_parenthesis; break;
        case '{': next_char(); token.type = Token_type::Open_brace; break;
        case '}': next_char(); token.type = Token_type::Close_brace; break;
        case '~': next_char(); token.type = Token_type::Complement; break;
        case '?': next_char(); token.type = Token_type::Question_mark; break;
        case ':': next_char(); token.type = Token_type::Colon; break;

            // compound tokens
        case '*': next_char();
            if (current_char == '=') {next_char(); token.type = Token_type::Compound_multiply;}
            else token.type = Token_type::Times;
            break;
        case '/': next_char();
            if (current_char == '=') {next_char(); token.type = Token_type::Compound_divide;}
            else token.type = Token_type::Slash;
            break;
        case '%': next_char();
            if (current_char == '=') {next_char(); token.type = Token_type::Compound_remainder;}
            else token.type = Token_type::Remainder;
            break;
        case '^': next_char();
            if (current_char == '=') {next_char(); token.type = Token_type::Compound_xor;}
            else token.type = Token_type::Bit_xor;
            break;
        case '+': next_char();
            if (current_char == '+') {next_char(); token.type = Token_type::Increment;}
            else if (current_char == '=') {next_char(); token.type = Token_type::Compound_add;}
            else token.type = Token_type::Plus;
            break;
        case '-': next_char();
            if (current_char == '-') {next_char(); token.type = Token_type::Decrement;}
            else if (current_char == '=') {next_char(); token.type = Token_type::Compound_subtract;}
            else token.type = Token_type::Minus;
            break;
        case '<': next_char();
            if (current_char == '<') {
                next_char(); token.type = Token_type::Shift_left;
                if (current_char == '=') {next_char(); token.type = Token_type::Compound_shiftl;}
            }
            else if (current_char == '=') {next_char(); token.type = Token_type::Less_equal;}
            else token.type = Token_type::Less;
            break;
        case '>': next_char();
            if (current_char == '>') {
                next_char(); token.type = Token_type::Shift_right;
                if (current_char == '=') {next_char(); token.type = Token_type::Compound_shiftr;}
            }
            else if (current_char == '=') {next_char(); token.type = Token_type::Greater_equal;}
            else token.type = Token_type::Greater;
            break;
        case '=': next_char();
            if (current_char == '=') {next_char(); token.type = Token_type::Equal;}
            else token.type = Token_type::Assign;
            break;
        case '&': next_char();
            if (current_char == '&') {next_char(); token.type = Token_type::And;}
            else if (current_char == '=') {next_char(); token.type = Token_type::Compound_and;}
            else token.type = Token_type::Bit_and;
            break;
        case '|': next_char();
            if (current_char == '|') {next_char(); token.type = Token_type::Or;}
            else if (current_char == '=') {next_char(); token.type = Token_type::Compound_or;}
            else token.type = Token_type::Bit_or;
            break;
        case '!': next_char();
            if (current_char == '=') {next_char(); token.type = Token_type::Not_equal;}
            else token.type = Token_type::Not;
            break;

        // unknown / unexpected character, return error token
        default:
            token.val += current_char;
            diag.report_issue(Severity::Error, token, Error_kind::Unknown_token);
            error_given = true;
            next_char();
            token.type = Token_type::None;
            break;
    }

    // if type is still 'None', something went wrong
    if (token.type == Token_type::None and not error_given)
        diag.report_issue(Severity::Error, token, Error_kind::Unknown_token);

    return token;
}

/**
 * This function reads from the input until it finds a character that cannot belong to an identifier or keyword. It then
 * sets the correct type (and value) of the token.
 * @param [inout] token Token will be initialized to identifier/keyword to be returned
 */
void Lexer::read_name(Token& token) {
    token.val.clear();

    // valid identifier characters: [a-zA-Z0-9_]
    while (std::isalnum(current_char) || current_char == '_') {
        token.val += current_char;
        if (next_char() != 0 && offset >= source.size()) break;
    }

    // check if it is a keyword
    static constexpr auto begin = std::begin(keywords);
    static constexpr auto end = std::end(keywords);
    if (const auto it =  std::lower_bound(begin, end, token.val);
                   it != end and *it == token.val) {
        token.type = keyword_types[static_cast<int>(it - begin)];
        token.val.clear();
    } else token.type = Token_type::Identifier; // not a keyword
}

/**
 * This function reads from the input until it finds a character that cannot belong to a number constant. It then sets
 * the correct type and value of the token.
 * @param [inout] token Token will be set to number constant
 */
void Lexer::read_number(Token& token) {
    token.val.clear();
    token.type = Token_type::Number_constant;

    // valid characters: [0-9]
    while (std::isdigit(current_char)) {
        token.val += current_char;
        if (next_char() != 0 || offset >= source.size()) break;
    }
}

/**
 * Debugging function that will tokenize the whole source code and print the tokens to std::cout.
 */
void Lexer::tokenize_all() {
    int i = 1;
    Token token;
    do {
        token = next();
        std::cout << i++ << ": " << token << "\n";
    } while (token.type != Token_type::EOF_);
}