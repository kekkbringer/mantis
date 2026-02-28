//
// Created by dominik on 7/11/25.
//

#include "tac_generator.hpp"

#include <cassert>

tac::Value Tac_generator::translate_expression(const ast::Expression_ptr &expr, std::vector<tac::Instruction_ptr> &insts) {
    tac::Value dst;

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, ast::Constant_ptr>) {
            dst = tac::Constant(arg->val);

        // conditional expression
        } else if constexpr (std::is_same_v<T, ast::Conditional_ptr>) {
            dst = tac::Variable(make_temporary());
            const auto e2_label = make_label("conditional_e2");
            const auto end_label = make_label("conditional_end");

            auto cond = translate_expression(arg->condition, insts);
            insts.emplace_back(std::make_unique<tac::Jump_if_zero>(cond, e2_label));
            auto e1 = translate_expression(arg->lhs, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(e1, dst));
            insts.emplace_back(std::make_unique<tac::Jump>(end_label));
            insts.emplace_back(std::make_unique<tac::Label>(e2_label));
            auto e2 = translate_expression(arg->rhs, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(e2, dst));
            insts.emplace_back(std::make_unique<tac::Label>(end_label));

        // unary expression
        } else if constexpr (std::is_same_v<T, ast::Unary_ptr>) {
            auto src = translate_expression(arg->expr, insts);
            dst = tac::Variable(make_temporary());
            auto tac_op = convert_unop(arg->op);
            insts.emplace_back(std::make_unique<tac::Unary>(tac_op, src, dst));

        // binary expression
        } else if constexpr (std::is_same_v<T, ast::Binary_ptr>) {
            dst = tac::Variable(make_temporary());

            // short-circuiting AND
            if (arg->op == ast::Binary_operator::And) {
                // prepare labels
                const auto false_label = make_label("false_and");
                const auto end_label = make_label("end_and");

                // true section (if no jump is taken)
                auto lhs = translate_expression(arg->lhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_zero>(tac::Jump_if_zero(lhs, false_label)));
                auto rhs = translate_expression(arg->rhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_zero>(tac::Jump_if_zero(rhs, false_label)));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(1), dst)));
                insts.emplace_back(std::make_unique<tac::Jump>(tac::Jump(end_label)));

                // false section
                insts.emplace_back(std::make_unique<tac::Label>(false_label));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(0), dst)));
                insts.emplace_back(std::make_unique<tac::Label>(end_label));

            // short-circuiting OR
            } else if (arg->op == ast::Binary_operator::Or) {
                // prepare labels
                const auto true_label = make_label("true_or");
                const auto end_label = make_label("end_or");

                // false section (if no jump is taken)
                auto lhs = translate_expression(arg->lhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_not_zero>(tac::Jump_if_not_zero(lhs, true_label)));
                auto rhs = translate_expression(arg->rhs, insts);
                insts.emplace_back(std::make_unique<tac::Jump_if_not_zero>(tac::Jump_if_not_zero(rhs, true_label)));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(0), dst)));
                insts.emplace_back(std::make_unique<tac::Jump>(tac::Jump(end_label)));

                // true section
                insts.emplace_back(std::make_unique<tac::Label>(true_label));
                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(tac::Constant(1), dst)));
                insts.emplace_back(std::make_unique<tac::Label>(end_label));

            // general binary expression
            } else {
                auto lhs = translate_expression(arg->lhs, insts);
                auto rhs = translate_expression(arg->rhs, insts);
                auto tac_op = convert_binop(arg->op);
                insts.emplace_back(std::make_unique<tac::Binary>(tac_op, lhs, rhs, dst));
            }

        // assignment
        } else if constexpr (std::is_same_v<T, ast::Assignment_ptr>) {
            auto lhs = translate_expression(arg->lhs, insts);
            auto rhs = translate_expression(arg->rhs, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(rhs, lhs)));
            dst = lhs; // assignment expressions return the new value of the variable

        // variable
        } else if constexpr (std::is_same_v<T, ast::Variable_ptr>) {
            dst = tac::Variable(arg->name);

        // post increment
        } else if constexpr (std::is_same_v<T, ast::Post_increment_ptr>) {
            dst = tac::Variable(make_temporary());
            auto val = translate_expression(arg->expr, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(val, dst)));
            insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Add, val, tac::Constant(1), val)));

        // post decrement
        } else if constexpr (std::is_same_v<T, ast::Post_decrement_ptr>) {
            dst = tac::Variable(make_temporary());
            auto val = translate_expression(arg->expr, insts);
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(val, dst)));
            insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Subtract, val, tac::Constant(1), val)));

        // pre increment
        } else if constexpr (std::is_same_v<T, ast::Pre_increment_ptr>) {
            dst = tac::Variable(make_temporary());
            auto val = translate_expression(arg->expr, insts);
            insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Add, val, tac::Constant(1), val)));
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(val, dst)));

        // pre decrement
        } else if constexpr (std::is_same_v<T, ast::Pre_decrement_ptr>) {
            dst = tac::Variable(make_temporary());
            auto val = translate_expression(arg->expr, insts);
            insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary(tac::Binary_operator::Subtract, val, tac::Constant(1), val)));
            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(val, dst)));

        // function call
        } else if constexpr (std::is_same_v<T, ast::Function_call_ptr>) {
            dst = tac::Variable(make_temporary());
            std::vector<tac::Value_ptr> tac_args;
            for (const auto& func_arg: arg->args) {
                tac_args.emplace_back(std::make_unique<tac::Value>(translate_expression(func_arg, insts)));
            }
            insts.emplace_back(std::make_unique<tac::Function_call>(arg->name, std::move(tac_args), std::make_unique<tac::Value>(dst)));

        } else if constexpr (std::is_same_v<T, std::monostate>) {
            // do nothing
        } else if constexpr (std::is_same_v<T, ast::Conditional_ptr>) {
            assert(false && "translate_expression not fully implemented yet");
        } else if constexpr (std::is_same_v<T, ast::Function_call_ptr>) {
            assert(false && "translate_expression not fully implemented yet");
        } else {
            assert(false && "internal error in translate_expression");
        }
    }, expr);

    return dst;
}
