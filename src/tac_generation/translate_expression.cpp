//
// Created by dominik on 7/11/25.
//

#include "tac_generator.hpp"
#include "util.hpp"

#include <cassert>

tac::Value Tac_generator::translate_expression(const ast::Expr* expr, std::vector<tac::Inst*> &insts) {
    switch (expr->kind) {
        case ast::Expr::Kind::Constant: {
            const auto* con = cast<const ast::Constant>(expr);
            return tac::Value::make_int(con->val);
        }

        case ast::Expr::Kind::Conditional: {
            const auto* condexpr = cast<const ast::Conditional>(expr);
            const auto e2_label  = string_table->intern(make_label("conditional_e2"));
            const auto end_label = string_table->intern(make_label("conditional_end"));
            const auto name_view = string_table->intern(make_temporary());
            const auto dst = tac::Value::make_variable(name_view);
            const auto cond = translate_expression(condexpr->cond, insts);
            insts.emplace_back(arena->allocate<tac::Jump_if_zero>(cond, e2_label));
            const auto e1 = translate_expression(condexpr->then, insts);
            insts.emplace_back(arena->allocate<tac::Copy>(e1, dst));
            insts.emplace_back(arena->allocate<tac::Jump>(end_label));
            insts.emplace_back(arena->allocate<tac::Label>(e2_label));
            const auto e2 = translate_expression(condexpr->else_, insts);
            insts.emplace_back(arena->allocate<tac::Copy>(e2, dst));
            insts.emplace_back(arena->allocate<tac::Label>(end_label));
            return dst;
        }

        case ast::Expr::Kind::Unary: {
            const auto* un = cast<const ast::Unary>(expr);
            auto src = translate_expression(un->operand, insts);
            const auto name_view = string_table->intern(make_temporary());
            auto dst = tac::Value::make_variable(name_view);
            auto tac_op = convert_unop(un->op);
            insts.emplace_back(arena->allocate<tac::Unary>(tac_op, src, dst));
            return dst;
        }

        case ast::Expr::Kind::Variable: {
            const auto* var = cast<const ast::Variable>(expr);
            return tac::Value::make_variable(var->name);
        }

        case ast::Expr::Kind::Assignment: {
            const auto* ass = cast<const ast::Assignment>(expr);
            auto lhs = translate_expression(ass->lhs, insts);
            auto rhs = translate_expression(ass->rhs, insts);
            insts.emplace_back(arena->allocate<tac::Copy>(rhs, lhs));
            return lhs; // assignment expressions return the new value of the variable
        }

        case ast::Expr::Kind::Function_call: {
            const auto* fun_call = cast<const ast::Function_call>(expr);
            const auto name_view = string_table->intern(make_temporary());
            auto dst = tac::Value::make_variable(name_view);
            std::vector<tac::Value> tmp_args;
            for (const auto& func_arg: fun_call->arguments()) {
                tmp_args.emplace_back(translate_expression(func_arg, insts));
            }
            // copy to arena
            auto* arg_ptr = arena->allocate_array<tac::Value>(tmp_args.size());
            std::ranges::copy(tmp_args, arg_ptr);
            auto* func_call = arena->allocate<tac::Function_call>(fun_call->name, arg_ptr, tmp_args.size(), dst);
            insts.emplace_back(func_call);
            return dst;
        }

        case ast::Expr::Kind::Inc_dec: {
            const auto* id_expr = cast<const ast::Inc_dec>(expr);
            const auto name_view = string_table->intern(make_temporary());
            auto dst = tac::Value::make_variable(name_view);
            auto val = translate_expression(id_expr->operand, insts);
            if (id_expr->is_prefix) {
                if (id_expr->is_increment)
                    insts.emplace_back(arena->allocate<tac::Binary>(tac::Binary_operator::Add, val, tac::Value::make_int(1), val));
                else
                    insts.emplace_back(arena->allocate<tac::Binary>(tac::Binary_operator::Subtract, val, tac::Value::make_int(1), val));
            }
            insts.emplace_back(arena->allocate<tac::Copy>(val, dst));
            if (not id_expr->is_prefix) {
                if (id_expr->is_increment)
                    insts.emplace_back(arena->allocate<tac::Binary>(tac::Binary_operator::Add, val, tac::Value::make_int(1), val));
                else
                    insts.emplace_back(arena->allocate<tac::Binary>(tac::Binary_operator::Subtract, val, tac::Value::make_int(1), val));
            }
            return dst;
        }

        case ast::Expr::Kind::Error: {
            assert(false && "translating error node...");
            break;
        }

        case ast::Expr::Kind::Binary: {
            const auto* bin_expr = cast<const ast::Binary>(expr);
            const auto name_view = string_table->intern(make_temporary());
            auto dst = tac::Value::make_variable(name_view);

            // short-circuiting AND
            if (bin_expr->op == ast::Binary_operator::And) {
                // prepare labels
                const auto false_label = string_table->intern(make_label("false_and"));
                const auto end_label   = string_table->intern(make_label("end_and"));

                // true section (if no jump is taken)
                auto lhs = translate_expression(bin_expr->lhs, insts);
                insts.emplace_back(arena->allocate<tac::Jump_if_zero>(lhs, false_label));
                auto rhs = translate_expression(bin_expr->rhs, insts);
                insts.emplace_back(arena->allocate<tac::Jump_if_zero>(rhs, false_label));
                insts.emplace_back(arena->allocate<tac::Copy>(tac::Value::make_int(1), dst));
                insts.emplace_back(arena->allocate<tac::Jump>(end_label));

                // false section
                insts.emplace_back(arena->allocate<tac::Label>(false_label));
                insts.emplace_back(arena->allocate<tac::Copy>(tac::Value::make_int(0), dst));
                insts.emplace_back(arena->allocate<tac::Label>(end_label));

            // short-circuiting OR
            } else if (bin_expr->op == ast::Binary_operator::Or) {
                // prepare labels
                const auto true_label = string_table->intern(make_label("true_or"));
                const auto end_label  = string_table->intern(make_label("end_or"));

                // false section (if no jump is taken)
                auto lhs = translate_expression(bin_expr->lhs, insts);
                insts.emplace_back(arena->allocate<tac::Jump_if_not_zero>(lhs, true_label));
                auto rhs = translate_expression(bin_expr->rhs, insts);
                insts.emplace_back(arena->allocate<tac::Jump_if_not_zero>(rhs, true_label));
                insts.emplace_back(arena->allocate<tac::Copy>(tac::Value::make_int(0), dst));
                insts.emplace_back(arena->allocate<tac::Jump>(end_label));

                // true section
                insts.emplace_back(arena->allocate<tac::Label>(true_label));
                insts.emplace_back(arena->allocate<tac::Copy>(tac::Value::make_int(1), dst));
                insts.emplace_back(arena->allocate<tac::Label>(end_label));

            // general binary expression
            } else {
                auto lhs = translate_expression(bin_expr->lhs, insts);
                auto rhs = translate_expression(bin_expr->rhs, insts);
                auto tac_op = convert_binop(bin_expr->op);
                insts.emplace_back(arena->allocate<tac::Binary>(tac_op, lhs, rhs, dst));
            }
            return dst;
        }
    }
    std::unreachable();
}
