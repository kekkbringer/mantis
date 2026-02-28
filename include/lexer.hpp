//
// Created by dominik on 7/3/25.
//

#ifndef LEXER_HPP
#define LEXER_HPP

#include <deque>
#include <string>

#include "source_manager.hpp"
#include "diagnostics_engine.hpp"
#include "token.hpp"

/**
 * The Lexer class takes input from the whole source code as a string and tokenizes it piece by piece to pass the
 * individual tokens to the parser.
 *
 * Upon encountering an invalid or otherwise unexpected token, it will report an error and return a token of
 * type 'None'. This allows the parser to potentially continue parsing the remaining part of the program.
 */
class Lexer {
private:
    Source_manager& source_manager;      ///< manages the source files
    const std::string& source;           ///< reference to the source code
    Diagnostics_engine& diag;            ///< responsible for error, warning and note reporting
    size_t line = 1, col = 0;            ///< line/column number
    size_t offset = 0;                   ///< global offset from start of source code
    char current_char = '\0';            ///< tracks the current character during lexing
    std::deque<Token> look_ahead_buffer; ///< keep a buffer to be able to look ahead further than one token

    int next_char();
    void read_name(Token& token);
    void read_number(Token& token);

    Token read_token();

public:
    Lexer(Source_manager& sm, Diagnostics_engine& de) : source_manager(sm), source(sm.get_source()), diag(de)
    {
        current_char = source.empty() ? '\0' : source[0];
    }

    Token next();
    Token peek(size_t n=1);

    // debug function
    void tokenize_all();
};

#endif // LEXER_HPP