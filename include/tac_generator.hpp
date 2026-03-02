//
// Created by dominik on 7/8/25.
//

#ifndef TAC_GENERATOR_HPP
#define TAC_GENERATOR_HPP

#include <cassert>
#include <utility>
#include <optional>

#include "ast.hpp"
#include "symbol_table.hpp"
#include "tac.hpp"
#include "arena_allocator.hpp"
#include "string_table.hpp"

/**
 * The Tac_generator is initialized with an AST program tree and can translate it to a tree containing only three
 * address code (TAC). It will generate temporary variables to save each intermediate results. The final tree is closer
 * to assembly code but still machine independent. Therefore, most optimizations that do not require knowledge about
 * the CPU will be carried out here.
 */
class Tac_generator {
private:
    const ast::Program* prog;    ///< AST program tree to be translated to the TAC
    Scope* file_scope = nullptr; ///< Symbol table (Pointer to file/global scope
    int tmp_counter = 0;         ///< keeps track of the number of temporary variables for name generation
    int label_counter = 0;       ///< keeps track of the number of internal labels for name generation
    ArenaAllocator* arena;       ///< pointer to the arena allocator
    StringTable* string_table;   ///< pointer to the string table to intern identifier

    tac::Program* gen_program();
    void traverse_scope(Scope* scope, std::vector<tac::Top_level>& tac_tl);

    std::optional<tac::Top_level> translate_declaration(const ast::Decl* decl);
    tac::Function* translate_function_decl(const ast::Func_decl* fdecl);
    void translate_block_item(const ast::Block_item* bitem, std::vector<tac::Inst*> &insts);
    void translate_statement(const ast::Stmt* stmt, std::vector<tac::Inst*> &insts);
    tac::Value translate_expression(const ast::Expr* expr, std::vector<tac::Inst*> &insts);

    /**
     * This function will generate a new, unique name for a temporary variable.
     * @return name of a new temporary variable
     */
    std::string make_temporary() { return ".tmp." + std::to_string(tmp_counter++); }

    /**
     * This function will generate a new, unique name for a label.
     * @return name of a new label
     */
    std::string make_label(const std::string &s) { return ".label." + s + std::to_string(tmp_counter++); }

    static tac::Unary_operator convert_unop(const ast::Unary_operator unop) {
        switch (unop) {
            case ast::Unary_operator::Complement: return tac::Unary_operator::Complement;
            case ast::Unary_operator::Negate: return tac::Unary_operator::Negate;
            case ast::Unary_operator::Not: return tac::Unary_operator::Not;
        }
        return tac::Unary_operator::Not;
    }

    static tac::Binary_operator convert_binop(const ast::Binary_operator binop) {
        switch (binop) {
            case ast::Binary_operator::Add:              return tac::Binary_operator::Add;
            case ast::Binary_operator::Subtract:         return tac::Binary_operator::Subtract;
            case ast::Binary_operator::Multiply:         return tac::Binary_operator::Multiply;
            case ast::Binary_operator::Divide:           return tac::Binary_operator::Divide;
            case ast::Binary_operator::Remainder:        return tac::Binary_operator::Remainder;
            case ast::Binary_operator::Bit_and:          return tac::Binary_operator::Bit_and;
            case ast::Binary_operator::Bit_or:           return tac::Binary_operator::Bit_or;
            case ast::Binary_operator::Bit_xor:          return tac::Binary_operator::Bit_xor;
            case ast::Binary_operator::Shift_left:       return tac::Binary_operator::Shift_left;
            case ast::Binary_operator::Shift_right:      return tac::Binary_operator::Shift_right;
            case ast::Binary_operator::Equal:            return tac::Binary_operator::Equal;
            case ast::Binary_operator::Not_equal:        return tac::Binary_operator::Not_equal;
            case ast::Binary_operator::Less_than:        return tac::Binary_operator::Less_than;
            case ast::Binary_operator::Less_or_equal:    return tac::Binary_operator::Less_or_equal;
            case ast::Binary_operator::Greater_than:     return tac::Binary_operator::Greater_than;
            case ast::Binary_operator::Greater_or_equal: return tac::Binary_operator::Greater_or_equal;
            default: assert(false && "binop conversion");
        }
        std::unreachable();
    }

public:
    explicit Tac_generator(const ast::Program* prog, Scope* fs, ArenaAllocator* aa, StringTable* st)
        : prog(prog), file_scope(fs), arena(aa), string_table(st) {}

    /**
     * Main routine that translates a program from a general abstract syntax tree to a three address code (TAC)
     * @return program translated into TAC
     */
    tac::Program* gen() { return gen_program(); }

    static void print_tac(const tac::Program* prog);
};

#endif //TAC_GENERATOR_HPP
