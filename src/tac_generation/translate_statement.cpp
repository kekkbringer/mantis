#include "tac_generator.hpp"
#include "util.hpp"

#include <cassert>
#include <iostream>

void Tac_generator::translate_statement(const ast::Stmt* stmt, std::vector<tac::Inst*> &insts) {
    switch (stmt->kind) {
        case ast::Stmt::Kind::Return: {
            const auto ret_stmt = cast<const ast::Return>(stmt);
            auto expr = translate_expression(ret_stmt->expr, insts);
            insts.emplace_back(arena->allocate<tac::Return>(expr));
            break;
        }

        case ast::Stmt::Kind::If: {
            const auto if_stmt = cast<const ast::If>(stmt);
            auto cond = translate_expression(if_stmt->condition, insts);
            const auto else_label = string_table->intern(make_label("else_if"));
            insts.emplace_back(arena->allocate<tac::Jump_if_zero>(cond, else_label));
            translate_statement(if_stmt->then, insts);

            // else present
            if (if_stmt->else_ != nullptr) {
                const auto end_label = string_table->intern(make_label("end_else_if"));
                insts.emplace_back(arena->allocate<tac::Jump>(end_label));
                insts.emplace_back(arena->allocate<tac::Label>(else_label));
                translate_statement(if_stmt->else_, insts);
                insts.emplace_back(arena->allocate<tac::Label>(end_label));
            } else {
                insts.emplace_back(arena->allocate<tac::Label>(else_label));
            }
            break;
        }

        case ast::Stmt::Kind::Compound: {
            const auto* compound_stmt = cast<const ast::Compound>(stmt);
            for (const auto& bitem: compound_stmt->block->item_span()) {
                translate_block_item(&bitem, insts);
            }
            break;
        }

        case ast::Stmt::Kind::Break: {
            const auto break_stmt = cast<const ast::Break>(stmt);
            const auto label = string_table->intern("break" + std::string(break_stmt->tag));
            insts.emplace_back(arena->allocate<tac::Jump>(label));
            break;
        }

        case ast::Stmt::Kind::Continue: {
            const auto con_stmt = cast<const ast::Continue>(stmt);
            const auto label = string_table->intern("continue" + std::string(con_stmt->tag));
            insts.emplace_back(arena->allocate<tac::Jump>(label));
            break;
        }

        case ast::Stmt::Kind::While: {
            const auto while_stmt = cast<const ast::While>(stmt);
            const auto continue_label = string_table->intern("continue" + std::string(while_stmt->tag));
            const auto break_label = string_table->intern("break" + std::string(while_stmt->tag));

            insts.emplace_back(arena->allocate<tac::Label>(continue_label));
            auto cond = translate_expression(while_stmt->condition, insts);
            insts.emplace_back(arena->allocate<tac::Jump_if_zero>(cond, break_label));
            translate_statement(while_stmt->body, insts);
            insts.emplace_back(arena->allocate<tac::Jump>(continue_label));
            insts.emplace_back(arena->allocate<tac::Label>(break_label));
            break;
        }

        case ast::Stmt::Kind::Do_while: {
            const auto do_while_stmt = cast<const ast::Do_while>(stmt);
            const auto start_label = string_table->intern(make_label("do_while_start"));
            const auto continue_label = string_table->intern("continue" + std::string(do_while_stmt->tag));
            const auto break_label = string_table->intern("break" + std::string(do_while_stmt->tag));

            insts.emplace_back(arena->allocate<tac::Label>(start_label));
            translate_statement(do_while_stmt->body, insts);
            insts.emplace_back(arena->allocate<tac::Label>(continue_label));
            auto cond = translate_expression(do_while_stmt->condition, insts);
            insts.emplace_back(arena->allocate<tac::Jump_if_not_zero>(cond, start_label));
            insts.emplace_back(arena->allocate<tac::Label>(break_label));
            break;
        }

        case ast::Stmt::Kind::For: {
            const auto* for_stmt = cast<const ast::For>(stmt);
            const auto start_label = string_table->intern(make_label("for_start"));
            const auto continue_label = string_table->intern("continue" + std::string(for_stmt->tag));
            const auto break_label = string_table->intern("break" + std::string(for_stmt->tag));

            // init first
            const auto& finit = for_stmt->init;
            if (finit.kind == ast::For_init::Kind::Decl) {
                // translation is only needed if the variable declaration includes initialization
                if (finit.var_decl->init != nullptr) {
                    const auto init_val = translate_expression(finit.var_decl->init, insts);
                    insts.emplace_back(arena->allocate<tac::Copy>(init_val, tac::Value::make_variable(finit.var_decl->name)));
                }
            } else if (finit.kind == ast::For_init::Kind::Expr) {
                if (finit.expr != nullptr) translate_expression(finit.expr, insts);
            }

            insts.emplace_back(arena->allocate<tac::Label>(start_label));
            if (for_stmt->condition != nullptr) {
                auto cond = translate_expression(for_stmt->condition, insts);
                insts.emplace_back(arena->allocate<tac::Jump_if_zero>(cond, break_label));
            }
            translate_statement(for_stmt->body, insts);
            insts.emplace_back(arena->allocate<tac::Label>(continue_label));

            // post
            if (for_stmt->post != nullptr) translate_expression(for_stmt->post, insts);

            insts.emplace_back(arena->allocate<tac::Jump>(start_label));
            insts.emplace_back(arena->allocate<tac::Label>(break_label));
            break;
        }

        case ast::Stmt::Kind::Null: {
            ; // fittingly this is a (redundant) null statement to translate a null statement
            break;
        }

        case ast::Stmt::Kind::Labeled: {
            const auto* lab_stmt = cast<const ast::Labeled>(stmt);
            insts.emplace_back(arena->allocate<tac::Label>(lab_stmt->label));
            translate_statement(lab_stmt->stmt, insts);
            break;
        }

        case ast::Stmt::Kind::Goto: {
            const auto* goto_stmt = cast<const ast::Goto>(stmt);
            insts.emplace_back(arena->allocate<tac::Jump>(goto_stmt->label));
            break;
        }

        case ast::Stmt::Kind::Switch: {
            const auto switch_stmt = cast<const ast::Switch>(stmt);
            // translate expr
            auto expr = translate_expression(switch_stmt->expr, insts);
            // loop over cases
            for (const int& i: switch_stmt->case_nums()) {
                // compare expr with case
                const auto name_view = string_table->intern(make_temporary());
                auto cond = tac::Value::make_variable(name_view);
                insts.emplace_back(arena->allocate<tac::Binary>(tac::Binary_operator::Equal, expr, tac::Value::make_int(i), cond));
                const auto label = string_table->intern(".case." + std::to_string(i) + "." + std::string(switch_stmt->tag));
                insts.emplace_back(arena->allocate<tac::Jump_if_not_zero>(cond, label));
            }
            const auto break_label   = string_table->intern("break" + std::string(switch_stmt->tag));
            const auto default_label = string_table->intern(".default." + std::string(switch_stmt->tag));
            if (switch_stmt->has_default) {
                insts.emplace_back(arena->allocate<tac::Jump>(default_label));
            } else {
                insts.emplace_back(arena->allocate<tac::Jump>(break_label));
            }
            translate_statement(switch_stmt->body, insts);
            insts.emplace_back(arena->allocate<tac::Label>(break_label));
            break;
        }

        case ast::Stmt::Kind::Switch_label: {
            const auto* sw_label = cast<const ast::Switch_label>(stmt);
                insts.emplace_back(arena->allocate<tac::Label>(sw_label->tag));
                translate_statement(sw_label->stmt, insts);
            break;
        }

        case ast::Stmt::Kind::Expr: {
            const auto* expr_stmt = cast<ast::Expr_stmt const>(stmt);
            translate_expression(expr_stmt->expr, insts);
        }

        case ast::Stmt::Kind::Error: {
            break;
        }
    }
}
