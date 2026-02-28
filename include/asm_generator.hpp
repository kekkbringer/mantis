//
// Created by dominik on 7/11/25.
//

#ifndef ASM_GENERATION_HPP
#define ASM_GENERATION_HPP

#include "asm_nodes.hpp"
#include "tac.hpp"
#include "symbol_table.hpp"

#include <string>

/**
 * The Asm_generator can be initialized with a TAC program tree. Then, the generator can be used to translate the TAC
 * program to a valid assembly program tree. This process is divided into several passes:
 *
 * In the first pass, the main part of the translation is carried out and already results in an assembly program tree.
 * This program is not yet valid, as it contains pseudo register used in the translation process and might also include
 * illegal operands in instructions (e.g. a mov instruction that takes two memory addresses).
 *
 * In the second pass, all pseudo register are replace by address on the stack or with a corresponding label of a symbol
 * like a static variable. Additionally, the total stack space of each function is calculated and set to be allocated in
 * the code emission step later.
 *
 * In the third pass, illegal instruction calls are fix. One example are mov instructions that have stack addresses as
 * both operands. These instructions are then modified and new instructions are inserted into the list, that e.g. first
 * move one of the operands into an auxiliary register.
 */
class Asm_generator {
private:
    tac::Program tac_prog;       ///< TAC program tree that will be translated to an assembly tree
    Scope* file_scope = nullptr; ///< Symbol table (Pointer to file/global scope)

    // translation routines
    void translate_program(assem::Program& prog);
    void translate_top_level(assem::Program& prog, const tac::Top_level_ptr& tlp);
    assem::Function translate_function(const tac::Function_ptr& fp);
    void translate_instructions(std::vector<assem::Instruction>& ainsts, const std::vector<tac::Instruction_ptr>& tinsts);
    void translate_return(std::vector<assem::Instruction>& ainsts, const tac::Return_ptr& rp);
    void translate_unary(std::vector<assem::Instruction>& ainsts, const tac::Unary_ptr& up);
    void translate_binary(std::vector<assem::Instruction>& ainsts, const tac::Binary_ptr& bp);
    assem::Operand translate_value(const tac::Value_ptr& vp);
    void translate_function_call(const tac::Function_call_ptr& fun_call, std::vector<assem::Instruction>& ainsts);
    void preg_to_stack(assem::Operand& op, std::unordered_map<std::string, int>& map, int& stack_top);
    void pseudo_reg_inst(assem::Instruction& inst, std::unordered_map<std::string, int>& map, int& stack_top);

    // modification routines
    void replace_pseudo_regs(assem::Program& prog);
    void replace_double_stack(assem::Program& prog);

public:
    explicit Asm_generator(tac::Program& tp, Scope* fs) : tac_prog(std::move(tp)), file_scope(fs) {}

    // driver of the translation process
    assem::Program gen();

    // debug function that prints the whole asm tree
    static void print_asm(const assem::Program& prog);
};

#endif //ASM_GENERATION_HPP
