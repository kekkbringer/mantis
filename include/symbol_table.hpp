//
// Created by dominik on 8/3/25.
//

#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <unordered_map>
#include <cassert>
#include <iostream>
#include <string>

#include "type_system.hpp"
#include "ast.hpp"

struct Symbol {
    std::string name;
    std::string unique_name;
    std::shared_ptr<Type> type = nullptr;
    ast::Decl* decl;
    enum class Linkage {None, Internal, External} linkage = Linkage::None;
    enum class Storage_duration {Automatic, Static} storage_duration = Storage_duration::Automatic;
    enum class Init {Tentative, Initial, None} init;
    enum class Kind {Var, Func, Struct, Enum, Typedef} kind;

    [[nodiscard]] bool has_linkage () const {
        return linkage != Linkage::None;
    }

    // print helper
    static std::string to_string(Init i) {
        switch (i) {
            case Init::Tentative: return "Tentative";
            case Init::Initial: return "Initial";
            case Init::None: return "None";
        }
        return "###";
    }

    static std::string to_string(const Kind k) {
        switch (k) {
            case Kind::Var: return "Var";
            case Kind::Func: return "Func";
            case Kind::Struct: return "Struct";
            case Kind::Enum: return "Enum";
            case Kind::Typedef: return "Typedef";
        }
        return "###";
    }

    static std::string to_string(const Linkage l) {
        switch (l) {
            case Linkage::None: return "None";
            case Linkage::Internal: return "Internal";
            case Linkage::External: return "External";
        }
        return "###";
    }

    static std::string to_string(const Storage_duration d) {
        switch (d) {
            case Storage_duration::Automatic: return "Automatic";
            case Storage_duration::Static: return "Static";
        }
        return "###";
    }

    [[nodiscard]] std::string to_string() const {
        std::string msg = "name: " + name + "\n"
                        + "unique_name: " + unique_name + "\n"
                        //+ "type: " + "NOT YET IMPLEMENTED" + "\n"
                        //+ "decl: " + "NOT YET IMPLEMENTED" + "\n"
                        + "linkage: " + to_string(linkage) + "\n"
                        + "storage duration: " + to_string(storage_duration) + "\n"
                        + "init: " + to_string(init) + "\n"
                        + "kind: " + to_string(kind) + "\n";
        return msg;
    }
};

class Scope {
public:
    Scope* parent = nullptr;
    std::vector<std::unique_ptr<Scope>> children; // a Scope owns all of its children
    std::unordered_map<std::string, Symbol> symbols;

    explicit Scope(Scope* parent = nullptr) : parent(parent) {}

    static std::string mangle_name(const std::string& name, size_t& mangle_counter) {
        return ".ma." + std::to_string(mangle_counter++) + "." + name;
    }

    /**
     * Declares the symbol in the current scope by placing it in the symbol table without mangling its name.
     * @param sym symbol to declare
     * @return 'true' if symbol was not already in table of this scope, 'false' otherwise
     */
    bool declare(Symbol& sym) {
        auto [it, ok] = symbols.emplace(sym.name, sym); // .second is false if already in map
        return ok;
    }

    /**
     * Declares the symbol in the current scope by placing it in the symbol table while mangling its name.
     * @param sym symbol to name-mangle and declare
     * @param mangle_counter counter to track unique name mangling
     * @return 'true' if symbol was not already in table of this scope, 'false' otherwise
     */
    bool declare(Symbol& sym, size_t& mangle_counter) {
        sym.unique_name = mangle_name(sym.name, mangle_counter);
        auto [it, ok] = symbols.emplace(sym.name, sym); // .second is false if already in map
        return ok;
    }

    /**
     * Search for symbol in whole symbol table?
     * @param name name to search for
     * @return pointer to the symbol of 'name' if found, 'nullptr' otherwise
     */
    Symbol* lookup(const std::string& name) {
        for (Scope* s=this; s!=nullptr; s=s->parent) {
            auto it = s->symbols.find(name);
            if (it != s->symbols.end())
                return &it->second;
        }
        return nullptr;
    }

    /**
     * Search for symbol with 'name' in this scope and all children.
     * @param name name to search for
     * @return pointer to the symbol of 'name' if found, 'nullptr' otherwise
     */
    Symbol* find_sym(const std::string& name) {
        // find global scope
        if (parent != nullptr) return parent->find_sym(name);

        // search this
        auto it = symbols.find(name);

        if (it != symbols.end()) return &it->second;

        // iterate over children and return first match
        for (const auto& child: children) {
            it = child->symbols.find(name);
            if (it != child->symbols.end()) return &it->second;
        }

        return nullptr;
    }

    /**
     * Search for symbol with 'name' and 'unique_name' in this scope and all children.
     * @param name name to search for
     * @param unique_name unique/mangled name to check in addition to name
     * @return pointer to the symbol of 'name' if found, 'nullptr' otherwise
     */
    Symbol* find_sym(const std::string& name, const std::string& unique_name) {
        // find global scope
        if (parent != nullptr) return parent->find_sym(name, unique_name);

        // search this
        auto it = symbols.find(name);
        if (it != symbols.end())
            if (it->second.unique_name == unique_name)
                return &it->second;

        // iterate over children and return first match
        for (const auto& child: children) {
            //return child->find_sym(name, unique_name);
            it = child->symbols.find(name);
            if (it != child->symbols.end())
                if (it->second.unique_name == unique_name)
                    return &it->second;
        }

        return nullptr;
    }

    // looks for the name in all scopes
    Symbol* find_global(const std::string& name) {
        // find global scope
        if (parent != nullptr) return parent->find_global(name);
        else return find_sym(name);
    }

    Symbol* descent_search(const std::string& name, const std::string& unique_name) {
        // search in this scope
        const auto it = symbols.find(name);
        if (it != symbols.end()) {
            if (it->second.unique_name == unique_name) {
                return &it->second;
            }
        }

        // search children
        for (const auto& child: children) {
            Symbol* sym = child->descent_search(name, unique_name);
            if (sym != nullptr) return sym;
        }

        return nullptr;
    }

    // looks for the name+unique_name in all scopes
    Symbol* find_global(const std::string& name, const std::string& unique_name) {
        // find global scope
        if (parent != nullptr) return parent->find_global(name, unique_name);
        else return descent_search(name, unique_name);
    }

    Symbol* any_external_linkage(const std::string& name) {
        // check this scope
        const auto it = symbols.find(name);
        if (it != symbols.end() and it->second.linkage == Symbol::Linkage::External)
            return &it->second;

        // loop over all children
        for (const auto& child: children) {
            Symbol* sym = child->any_external_linkage(name);
            if (sym != nullptr) return sym;
        }

        return nullptr;
    }

    // look only in the file scope
    Symbol* lookup_file_scope(const std::string& name) {
        if (parent != nullptr) return parent->lookup_file_scope(name);

        // only check file scope
        const auto it =  symbols.find(name);
        if (it != symbols.end()) return &it->second;
        return nullptr;
    }

    // returns a pointer to the file scope
    Scope* get_file_scope() {
        if (parent == nullptr) return this;
        else return parent->get_file_scope();
    }

    bool is_global_scope() const { return (parent == nullptr); }
};

/**
 * @brief Returns the location in the source code of the symbol in question.
 */
inline Source_location get_location(const Symbol* sym) {
    return sym->decl->loc;
}

#endif //SYMBOL_TABLE_HPP
