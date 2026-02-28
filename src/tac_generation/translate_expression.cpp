//
// Created by dominik on 7/11/25.
//

#include "tac_generator.hpp"
#include "util.hpp"

#include <cassert>

tac::Value Tac_generator::translate_expression(const ast::Expr* expr, std::vector<tac::Instruction_ptr> &insts) {
    tac::Value dst;

    switch (expr->kind) {
        case ast::Expr::Kind::Constant: {
            const auto* con = cast<const ast::Constant>(expr);
            dst = tac::Constant(con->val);
            break;
        }

        case ast::Expr::Kind::Conditional: {
            const auto* condexpr = cast<const ast::Conditional>(expr);
            dst = tac::Variable(make_temporary());
            const auto e2_label = make_label("conditional_e2");
            const auto end_label = make_label("conditional_end");

            auto cond = translate_expression(condexpr->cond, insts);
            insts.emplace_back(std::make_unique<tac::Jump_if_zero>(cond, e2_label));
            auto e1 = translate_expression(condexpr->then, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(e1, dst));
            insts.emplace_back(std::make_unique<tac::Jump>(end_label));
            insts.emplace_back(std::make_unique<tac::Label>(e2_label));
            auto e2 = translate_expression(condexpr->else_, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(e2, dst));
            insts.emplace_back(std::make_unique<tac::Label>(end_label));
            break;
        }

        case ast::Expr::Kind::Unary: {
            const auto* un = cast<const ast::Unary>(expr);
            auto src = translate_expression(un->operand, insts);
            dst = tac::Variable(make_temporary());
            auto tac_op = convert_unop(un->op);
            insts.emplace_back(std::make_unique<tac::Unary>(tac_op, src, dst));
            break;
        }

        case ast::Expr::Kind::Variable: {
            const auto* var = cast<const ast::Variable>(expr);
            dst = tac::Variable(std::string(var->name));
            break;
        }

        case ast::Expr::Kind::Assignment: {
            const auto* ass = cast<const ast::Assignment>(expr);
            auto lhs = translate_expression(ass->lhs, insts);
            auto rhs = translate_expression(ass->rhs, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(rhs, lhs)));
            dst = lhs; // assignment expressions return the new value of the variable
            break;
        }

        case ast::Expr::Kind::Function_call: {
            const auto* fun_call = cast<const ast::Function_call>(expr);
            dst = tac::Variable(make_temporary());
            std::vector<tac::Value_ptr> tac_args;
            for (const auto& func_arg: fun_call->arguments()) {
                tac_args.emplace_back(std::make_unique<tac::Value>(translate_expression(func_arg, insts)));
            }
            insts.emplace_back(std::make_unique<tac::Function_call>(std::string(fun_call->name), std::move(tac_args), std::make_unique<tac::Value>(dst)));
            break;
        }

        case ast::Expr::Kind::Inc_dec: {
            const auto* id_expr = cast<const ast::Inc_dec>(expr);
            dst = tac::Variable(make_temporary());
            auto val = translate_expression(id_expr->operand, insts);
            if (id_expr->is_prefix) {
                if (id_expr->is_increment)
                    insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Add, val, tac::Constant(1), val)));
                else
                    insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Subtract, val, tac::Constant(1), val)));
            }
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(val, dst)));
            if (not id_expr->is_prefix) {
                if (id_expr->is_increment)
                    insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Add, val, tac::Constant(1), val)));
                else
                    insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Subtract, val, tac::Constant(1), val)));
            }
            break;
        }

        case ast::Expr::Kind::Error: {
            assert(false && "translating error node...");
            std::unreachable();
            break;
        }

        case ast::Expr::Kind::Binary: {
            const auto* bin_expr = cast<const ast::Binary>(expr);
            dst = tac::Variable(make_temporary());

            // short-circuiting AND
            if (bin_expr->op == ast::Binary_operator::And) {
                // prepare labels
                const auto false_label = make_label("false_and");
                const auto end_label = make_label("end_and");

                // true section (if no jump is taken)
                auto lhs = translate_expression(bin_expr->lhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_zero>(tac::Jump_if_zero(lhs, false_label)));
                auto rhs = translate_expression(bin_expr->rhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_zero>(tac::Jump_if_zero(rhs, false_label)));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(1), dst)));
                insts.emplace_back(std::make_unique<tac::Jump>(tac::Jump(end_label)));

                // false section
                insts.emplace_back(std::make_unique<tac::Label>(false_label));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(0), dst)));
                insts.emplace_back(std::make_unique<tac::Label>(end_label));

            // short-circuiting OR
            } else if (bin_expr->op == ast::Binary_operator::Or) {
                // prepare labels
                const auto true_label = make_label("true_or");
                const auto end_label = make_label("end_or");

                // false section (if no jump is taken)
                auto lhs = translate_expression(bin_expr->lhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_not_zero>(tac::Jump_if_not_zero(lhs, true_label)));
                auto rhs = translate_expression(bin_expr->rhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_not_zero>(tac::Jump_if_not_zero(rhs, true_label)));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(0), dst)));
                insts.emplace_back(std::make_unique<tac::Jump>(tac::Jump(end_label)));

                // true section
                insts.emplace_back(std::make_unique<tac::Label>(true_label));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(1), dst)));
                insts.emplace_back(std::make_unique<tac::Label>(end_label));

            // general binary expression
            } else {
                auto lhs = translate_expression(bin_expr->lhs, insts);
                auto rhs = translate_expression(bin_expr->rhs, insts);
                auto tac_op = convert_binop(bin_expr->op);
                insts.emplace_back(std::make_unique<tac::Binary>(tac_op, lhs, rhs, dst));
            }
            break;
        }
    }

    return dst;
}
