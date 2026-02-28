#include "tac_generator.hpp"

#include <cassert>
#include <iostream>

void Tac_generator::translate_statement(const ast::Statement_ptr &stmt, std::vector<tac::Instruction_ptr> &insts) {
    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;

        // return statement
        if constexpr (std::is_same_v<T, ast::Return_ptr>) {
            auto expr = translate_expression(arg->expr, insts);
            insts.emplace_back(std::make_unique<tac::Return>(expr));

        // for loop
        } else if constexpr (std::is_same_v<T, ast::For_ptr>) {
            const auto start_label = make_label("for_start");
            const auto continue_label = "continue" + arg->tag;
            const auto break_label = "break" + arg->tag;

            // init first
            std::visit([&]<class U0>(U0&& for_init) {
                using U = std::decay_t<U0>;
                if constexpr(std::is_same_v<U, ast::Declaration_ptr>) {
                    if (std::holds_alternative<ast::Variable_declaration_ptr>(for_init)) {
                        // translation is only needed if the variable declaration includes initialization
                        if (not std::holds_alternative<std::monostate>(std::get<ast::Variable_declaration_ptr>(for_init)->init)) {
                            auto init_val = translate_expression(std::get<ast::Variable_declaration_ptr>(for_init)->init, insts);
                            insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(init_val, tac::Variable(std::get<ast::Variable_declaration_ptr>(for_init)->name))));
                        }
                    }
                } else if constexpr(std::is_same_v<U, ast::Expression_ptr>) {
                    translate_expression(for_init, insts);
                }
            }, arg->init);

            insts.emplace_back(std::make_unique<tac::Label>(start_label));
            auto cond = translate_expression(arg->condition, insts);
            if (not std::holds_alternative<std::monostate>(arg->condition)) insts.emplace_back(std::make_unique<tac::Jump_if_zero>(cond, break_label));
            translate_statement(arg->body, insts);
            insts.emplace_back(std::make_unique<tac::Label>(continue_label));

            // post
            translate_expression(arg->post, insts);

            insts.emplace_back(std::make_unique<tac::Jump>(start_label));
            insts.emplace_back(std::make_unique<tac::Label>(break_label));

        // while loop
        } else if constexpr (std::is_same_v<T, ast::While_ptr>) {
            const auto continue_label = "continue" + arg->tag;
            const auto break_label = "break" + arg->tag;

            insts.emplace_back(std::make_unique<tac::Label>(continue_label));
            auto cond = translate_expression(arg->condition, insts);
            insts.emplace_back(std::make_unique<tac::Jump_if_zero>(cond, break_label));
            translate_statement(arg->body, insts);
            insts.emplace_back(std::make_unique<tac::Jump>(continue_label));
            insts.emplace_back(std::make_unique<tac::Label>(break_label));

        // do while loop
        } else if constexpr (std::is_same_v<T, ast::Do_while_ptr>) {
            const auto start_label = make_label("do_while_start");
            const auto continue_label = "continue" + arg->tag;
            const auto break_label = "break" + arg->tag;

            insts.emplace_back(std::make_unique<tac::Label>(start_label));
            translate_statement(arg->body, insts);
            insts.emplace_back(std::make_unique<tac::Label>(continue_label));
            auto cond = translate_expression(arg->condition, insts);
            insts.emplace_back(std::make_unique<tac::Jump_if_not_zero>(cond, start_label));
            insts.emplace_back(std::make_unique<tac::Label>(break_label));

        // break statement
        } else if constexpr (std::is_same_v<T, ast::Break_ptr>) {
            insts.emplace_back(std::make_unique<tac::Jump>("break" + arg->tag));

        // continue statement
        } else if constexpr (std::is_same_v<T, ast::Continue_ptr>) {
            insts.emplace_back(std::make_unique<tac::Jump>("continue" + arg->tag));

        // null statement
        } else if constexpr (std::is_same_v<T, ast::Null_ptr>) {
            ; // fittingly this is a (redundant) null statement to translate a null statement :P

        // expression statement
        } else if constexpr (std::is_same_v<T, ast::Expression_ptr>) {
            translate_expression(arg, insts);

        // goto statement
        } else if constexpr (std::is_same_v<T, ast::Goto_ptr>) {
            insts.emplace_back(std::make_unique<tac::Jump>(arg->label));

        // labeled statement
        } else if constexpr (std::is_same_v<T, ast::Labeled_ptr>) {
            insts.emplace_back(std::make_unique<tac::Label>(arg->label));
            translate_statement(arg->stmt, insts);

        // if statement
        } else if constexpr (std::is_same_v<T, ast::If_ptr>) {
            auto cond = translate_expression(arg->condition, insts);
            const auto else_label = make_label("else_if");
            insts.emplace_back(std::make_unique<tac::Jump_if_zero>(cond, else_label));
            translate_statement(arg->then, insts);

            // else present
            if (not std::holds_alternative<std::monostate>(arg->else_)) {
                const auto end_label = make_label("end_else_if");
                insts.emplace_back(std::make_unique<tac::Jump>(end_label));
                insts.emplace_back(std::make_unique<tac::Label>(else_label));
                translate_statement(arg->else_, insts);
                insts.emplace_back(std::make_unique<tac::Label>(end_label));
            } else {
                insts.emplace_back(std::make_unique<tac::Label>(else_label));
            }

        // compound statement
        } else if constexpr (std::is_same_v<T, ast::Compound_ptr>) {
            for (const auto& bitem: *arg->block) {
                translate_block_item(bitem, insts);
            }

        // case statement
        } else if constexpr (std::is_same_v<T, ast::Case_ptr>) {
            insts.emplace_back(std::make_unique<tac::Label>(arg->tag));
            translate_statement(arg->stmt, insts);

        // default statement
        } else if constexpr (std::is_same_v<T, ast::Default_ptr>) {
            insts.emplace_back(std::make_unique<tac::Label>(arg->tag));
            translate_statement(arg->stmt, insts);

        // switch statement
        } else if constexpr (std::is_same_v<T, ast::Switch_ptr>) {
            // translate expr
            auto expr = translate_expression(arg->expr, insts);
            // loop over cases
            for (const int& i: arg->cases) {
                // compare expr with case
                auto cond = tac::Variable(make_temporary());
                insts.emplace_back(std::make_unique<tac::Binary>(tac::Binary_operator::Equal, expr, tac::Constant(i), cond));
                insts.emplace_back(std::make_unique<tac::Jump_if_not_zero>(cond, ".case." + std::to_string(i) + "." + arg->tag));
            }
            const auto break_label = "break" + arg->tag;
            const auto default_label = ".default." + arg->tag;
            if (arg->has_default) {
                insts.emplace_back(std::make_unique<tac::Jump>(default_label));
            } else {
                insts.emplace_back(std::make_unique<tac::Jump>(break_label));
            }
            translate_statement(arg->body, insts);
            insts.emplace_back(std::make_unique<tac::Label>(break_label));

        } else {
            assert(false && "translate statement ist noch nicht fertig geschrieben");
        }
    },stmt);
}
