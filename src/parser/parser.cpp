#include "parser.hpp"

/**
 * Main routine to start the parser, recursively calls parser functions to parse different language constructs.
 */
ast::Program Parser::parse() {
    return parse_program();
}
