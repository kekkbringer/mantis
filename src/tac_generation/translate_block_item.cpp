//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"

#include <cassert>

void Tac_generator::translate_block_item(const ast::Block_item_ptr& bitem, std::vector<tac::Instruction_ptr>& insts) {
    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;

        // statement
        if constexpr (std::is_same_v<T, ast::Statement_ptr>) {
            translate_statement(arg, insts);

        // declaration
        } else if constexpr (std::is_same_v<T, ast::Declaration_ptr>) {
            std::visit([&]<class T1>(T1&& arg1) {
                using T = std::decay_t<T1>;

                // variable
                if constexpr (std::is_same_v<T, ast::Variable_declaration_ptr>) {
                    if (std::holds_alternative<ast::Variable_declaration_ptr>(arg)) {
                        // translation is only needed if the variable declaration includes initialization
                        if (not std::holds_alternative<std::monostate>(arg1->init)) {
                            // translation still needs to be skipped if it has internal linkage
                            if (arg1->storage_class != ast::Storage_class::Static) {
                                auto init_val = translate_expression(arg1->init, insts);
                                insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(init_val, tac::Variable(arg1->name))));
                            }
                        }
                    }

                // function
                } else if constexpr (std::is_same_v<T, ast::Function_declaration_ptr>) {
                }

            }, arg);
        }
    }, bitem);
}
