//
// Created by dominik on 2/13/26.
//

#include "tac_generator.hpp"
#include "util.hpp"

void Tac_generator::traverse_scope(Scope* scope, std::vector<tac::Top_level>& tac_tl) {
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
                    const auto unique_name_view = string_table->intern(sym.unique_name);
                    auto* ptr = arena->allocate<tac::Static_variable>(unique_name_view, sym.linkage == Symbol::Linkage::External, constant->val);
                    tac_tl.emplace_back(ptr);

                // tentative init
                } else if (sym.init == Symbol::Init::Tentative) {
                    const auto unique_name_view = string_table->intern(sym.unique_name);
                    auto* ptr = arena->allocate<tac::Static_variable>(unique_name_view, sym.linkage == Symbol::Linkage::External, 0);
                    tac_tl.emplace_back(ptr);
                }
            }
        }
    }

    // visit all children
    for (const auto& child: scope->children) {
        traverse_scope(child.get(), tac_tl);
    }
}
