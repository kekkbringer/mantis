//
// Created by dominik on 7/8/25.
//

#include "tac_generator.hpp"
#include "util.hpp"

#include <cassert>

void Tac_generator::translate_block_item(const ast::Block_item* bitem, std::vector<tac::Instruction_ptr>& insts) {
    // statement as block item
    if (bitem->kind == ast::Block_item::Kind::Stmt) {
        translate_statement(bitem->stmt, insts);

    // declaration as block item
    } else {
        // only need to translate a variable declaration
        if (bitem->decl->kind == ast::Decl::Kind::Var) {
            const auto* var_decl = cast<ast::Var_decl>(bitem->decl);
            // translation is only needed if the variable declaration includes an initialization
            if (var_decl->init != nullptr) {
                // translation still needs to be skipped if it has static storage duration
                if (var_decl->storage_class != ast::Decl::Storage_class::Static) {
                    auto init_val = translate_expression(var_decl->init, insts);
                    insts.emplace_back(std::make_unique<tac::Copy>(tac::Copy(init_val, tac::Variable(var_decl->name.data()))));
                }
            }
        }
    }
}
