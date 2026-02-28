//
// Created by dominik on 7/7/25.
//

#include "parser.hpp"
#include "tac_binary_operator.hpp"

/**
 * Return the precedence of the operator given as a string
 */
int binop_prec(const Token_type& tt) {
    switch (tt) {
        // * / %
        case Token_type::Times: case Token_type::Slash: case Token_type::Remainder: return 50;
        // + -
        case Token_type::Plus: case Token_type::Minus: return 45;
        // << >>
        case Token_type::Shift_left: case Token_type::Shift_right: return 40;
        // < > <= >=
        case Token_type::Less: case Token_type::Greater: case Token_type::Less_equal: case Token_type::Greater_equal: return 35;
        // == !=
        case Token_type::Equal: case Token_type::Not_equal: return 30;
        // &
        case Token_type::Bit_and: return 25;
        // ^
        case Token_type::Bit_xor: return 20;
        // |
        case Token_type::Bit_or: return 15;
        // &&
        case Token_type::And: return 10;
        // ||
        case Token_type::Or: return 5;
        // ?
        case Token_type::Question_mark: return 3;
        // = and compound binary operations
        case Token_type::Assign:
        case Token_type::Compound_add: case Token_type::Compound_subtract:
        case Token_type::Compound_multiply: case Token_type::Compound_divide: case Token_type::Compound_remainder:
        case Token_type::Compound_and: case Token_type::Compound_or: case Token_type::Compound_xor:
        case Token_type::Compound_shiftl: case Token_type::Compound_shiftr: return 2;
        default: return -100;
    }
}

/**
* Function to parse an expression with the following syntax in EBNF:
* <exp> ::= <factor> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>
* <factor> ::= <int> | <unop> <factor> | "(" <exp> ")" | <exp>
* <binop> ::= "-" | "+" | "*" | "/" | "%"
* @param min_prec minimum precedence of the next operator required for nesting
* @return unique_ptr to an expression node
*/
ast::Expression_ptr Parser::parse_expression(const int min_prec) {
    // left associativity
    auto lhs = parse_factor();
    // check for catastrophic error from parse_factor
    if (std::holds_alternative<ast::ErrorExpr_ptr>(lhs)) return lhs;

    // binary operator
    while (look_ahead.is_binary() and binop_prec(la_type) >= min_prec) {
        // save operator type and advance token stream
        const auto op_token = look_ahead;
        const auto& op = op_token.type;

        // assign is right associative
        if (op == Token_type::Assign) {
            // test if lhs is a valid lvalue
            // so far only variables are valid lvalues
            if (not std::holds_alternative<ast::Variable_ptr>(lhs)) {
                diag.report_issue(Severity::Error, current, Error_kind::Invalid_lvalue, "in assignment");
            }
            scan();

            auto rhs = parse_expression(binop_prec(current.type));
            // check for catastrophic error from parse_factor
            if (std::holds_alternative<ast::ErrorExpr_ptr>(rhs)) return rhs;
            lhs = std::make_unique<ast::Assignment>(std::move(lhs), std::move(rhs));

        // conditional expression ? :
        } else if (op == Token_type::Question_mark) {
            scan();
            auto mhs = parse_expression(0);
            // check for catastrophic error from parse_factor
            if (std::holds_alternative<ast::ErrorExpr_ptr>(mhs)) return mhs;
            scan(); // ':' token
            auto rhs = parse_expression(binop_prec(Token_type::Question_mark));
            // check for catastrophic error from parse_factor
            if (std::holds_alternative<ast::ErrorExpr_ptr>(rhs)) return rhs;
            lhs = std::make_unique<ast::Conditional>(std::move(lhs), std::move(mhs), std::move(rhs));

        // compound and true binary
        } else {
            // compound operator like '+=' or '<<='?
            if (op_token.is_compound()) {
                // test if lhs is a valid lvalue
                // so far only variables are valid lvalues
                if (not std::holds_alternative<ast::Variable_ptr>(lhs)) {
                    diag.report_issue(Severity::Error, current, Error_kind::Invalid_lvalue, "in compound assignment");
                }
                scan();

                // parse rhs of binary expression (right associative)
                auto rhs = parse_expression(binop_prec(current.type));
                // check for catastrophic error from parse_factor
                if (std::holds_alternative<ast::ErrorExpr_ptr>(rhs)) return rhs;

                // first, make operational expression
                ast::Binary_operator ast_op;
                switch (op) {
                    case Token_type::Compound_add:       ast_op = ast::Binary_operator::Add;         break;
                    case Token_type::Compound_subtract:  ast_op = ast::Binary_operator::Subtract;    break;
                    case Token_type::Compound_multiply:  ast_op = ast::Binary_operator::Multiply;    break;
                    case Token_type::Compound_divide:    ast_op = ast::Binary_operator::Divide;      break;
                    case Token_type::Compound_remainder: ast_op = ast::Binary_operator::Remainder;   break;
                    case Token_type::Compound_and:       ast_op = ast::Binary_operator::Bit_and;     break;
                    case Token_type::Compound_or:        ast_op = ast::Binary_operator::Bit_or;      break;
                    case Token_type::Compound_xor:       ast_op = ast::Binary_operator::Bit_xor;     break;
                    case Token_type::Compound_shiftl:    ast_op = ast::Binary_operator::Shift_left;  break;
                    case Token_type::Compound_shiftr:    ast_op = ast::Binary_operator::Shift_right; break;
                    default:
                        diag.report_issue(Severity::Error, current, Error_kind::Unknown, "binary compound operator");
                }
                ast::Variable_ptr var_copy;
                if (std::holds_alternative<ast::Variable_ptr>(lhs))
                    var_copy = std::make_unique<ast::Variable>(*std::get<ast::Variable_ptr>(lhs));
                rhs = std::make_unique<ast::Binary>(ast_op, std::move(var_copy), std::move(rhs));
                lhs = std::make_unique<ast::Assignment>(std::move(lhs), std::move(rhs));

            // normal binary operation (non-compound, non-assignment)
            } else {
                scan(); // operator
                // parse rhs of binary expression (left associative, hence prec+1)
                auto rhs = parse_expression(binop_prec(current.type) + 1);
                // check for catastrophic error from parse_factor
                if (std::holds_alternative<ast::ErrorExpr_ptr>(rhs)) return rhs;

                ast::Binary_operator ast_op;
                switch (op) {
                    case Token_type::Plus:          ast_op = ast::Binary_operator::Add;              break;
                    case Token_type::Minus:         ast_op = ast::Binary_operator::Subtract;         break;
                    case Token_type::Times:         ast_op = ast::Binary_operator::Multiply;         break;
                    case Token_type::Slash:         ast_op = ast::Binary_operator::Divide;           break;
                    case Token_type::Remainder:     ast_op = ast::Binary_operator::Remainder;        break;
                    case Token_type::Bit_and:       ast_op = ast::Binary_operator::Bit_and;          break;
                    case Token_type::Bit_or:        ast_op = ast::Binary_operator::Bit_or;           break;
                    case Token_type::Bit_xor:       ast_op = ast::Binary_operator::Bit_xor;          break;
                    case Token_type::Shift_left:    ast_op = ast::Binary_operator::Shift_left;       break;
                    case Token_type::Shift_right:   ast_op = ast::Binary_operator::Shift_right;      break;
                    case Token_type::And:           ast_op = ast::Binary_operator::And;              break;
                    case Token_type::Or:            ast_op = ast::Binary_operator::Or;               break;
                    case Token_type::Equal:         ast_op = ast::Binary_operator::Equal;            break;
                    case Token_type::Not_equal:     ast_op = ast::Binary_operator::Not_equal;        break;
                    case Token_type::Less:          ast_op = ast::Binary_operator::Less_than;        break;
                    case Token_type::Less_equal:    ast_op = ast::Binary_operator::Less_or_equal;    break;
                    case Token_type::Greater:       ast_op = ast::Binary_operator::Greater_than;     break;
                    case Token_type::Greater_equal: ast_op = ast::Binary_operator::Greater_or_equal; break;
                    case Token_type::Assign:        ast_op = ast::Binary_operator::Assign;           break;
                    default:
                        diag.report_issue(Severity::Error, current, Error_kind::Unknown, "binary operator");
                }

                // put lhs and rhs together
                lhs = std::make_unique<ast::Binary>(ast_op, std::move(lhs), std::move(rhs));
            }
        }
    }

    // return final expression that may be nested
    return lhs;
}
