//
// Created by dominik on 2/13/26.
//

#include "tac_generator.hpp"
#include "util.hpp"

void Tac_generator::traverse_scope(Scope* scope, tac::Program& tac_prog) {
    if (scope == nullptr) return;

    // loop over all symbols
    for (const auto& [name, sym]: scope->symbols) {
        if (sym.kind == Symbol::Kind::Var) {
            const auto* var_decl = cast<ast::Var_decl const>(sym.decl);
            if (sym.storage_duration == Symbol::Storage_duration::Static) {
                // actual initialization
                if (sym.init == Symbol::Init::Initial) {
                    // determine value of initial value
                    const auto* constant = cast<ast::Constant const>(var_decl->init);
                    tac_prog.emplace_back(std::make_unique<tac::Static_variable>(sym.unique_name, sym.linkage == Symbol::Linkage::External, constant->val));

                // tentative init
                } else if (sym.init == Symbol::Init::Tentative) {
                    tac_prog.emplace_back(std::make_unique<tac::Static_variable>(sym.unique_name, sym.linkage == Symbol::Linkage::External, 0));
                }
            }
        }
    }

    // visit all children
    for (const auto& child: scope->children) {
        traverse_scope(child.get(), tac_prog);
    }
}