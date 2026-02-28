//
// Created by dominik on 7/12/25.
//

#include <cassert>

#include "code_emitter.hpp"

/**
 * Emits the source code for an assembly instruction node. In the case of a return instruction, the function epilog will
 * be emitted first, before returning control to the caller.
 * @param i
 */
void Code_emitter::emit_instruction(const assem::Instruction& i) {
    std::visit([&]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;

        // mov
        if constexpr (std::is_same_v<T, assem::Mov>) {
            os << "\tmovl\t";
            emit_operand(arg.src);
            os << ", ";
            emit_operand(arg.dst);
            os << "\n";

        // ret (with function epilog)
        } else if constexpr (std::is_same_v<T, assem::Ret>) {
#ifdef DEBUG_MODE
    os << "\n# function epilog\n";
#endif
            os << "\tmovq\t%rbp, %rsp\n";
            os << "\tpopq\t%rbp\n";
            os << "\tret\n";

        // unary operator
        } else if constexpr (std::is_same_v<T, assem::Unary>) {
            if (arg.op == assem::Un_op::Neg) os << "\tnegl\t";
            else os << "\tnotl\t";
            emit_operand(arg.operand);
            os << "\n";

        // binary operator
        } else if constexpr (std::is_same_v<T, assem::Binary>) {
            if (arg.op == assem::Bin_op::Add) os << "\taddl\t";
            else if (arg.op == assem::Bin_op::Sub) os << "\tsubl\t";
            else if (arg.op == assem::Bin_op::Mult) os << "\timull\t";
            else if (arg.op == assem::Bin_op::And) os << "\tandl\t";
            else if (arg.op == assem::Bin_op::Or) os << "\torl\t";
            else if (arg.op == assem::Bin_op::Xor) os << "\txorl\t";
            else if (arg.op == assem::Bin_op::Sal) os << "\tsall\t";
            else if (arg.op == assem::Bin_op::Sar) os << "\tsarl\t";
            else assert(false && "binop emission");
            emit_operand(arg.src);
            os << ", ";
            emit_operand(arg.dst);
            os << "\n";

        // idiv
        } else if constexpr (std::is_same_v<T, assem::Idiv>) {
            os << "\tidivl\t";
            emit_operand(arg.operand);
            os << "\n";

        // sign extension
        } else if constexpr (std::is_same_v<T, assem::Cdq>) {
            os << "\tcdq\n";

        // cmp
        } else if constexpr (std::is_same_v<T, assem::Cmp>) {
            os << "\tcmpl\t";
            emit_operand(arg.lhs);
            os << ", ";
            emit_operand(arg.rhs);
            os << "\n";

        // Jmp
        } else if constexpr (std::is_same_v<T, assem::Jmp>) {
            os << "\tjmp\t.L" << arg.name << "\n";

        // JmpCC
        } else if constexpr (std::is_same_v<T, assem::JmpCC>) {
            os << "\tj" << to_string(arg.cc) << "\t.L" << arg.name << "\n";

        // label
        } else if constexpr (std::is_same_v<T, assem::Label>) {
            os << ".L" << arg.name << ":\n";

        // SetCC
        } else if constexpr (std::is_same_v<T, assem::SetCC>) {
            os << "\tset" << to_string(arg.cc) << "\t";
            emit_operand(arg.operand);
            os << "\n";

        // call
        } else if constexpr (std::is_same_v<T, assem::Call>) {
            os << "\tcall\t" << arg.name << "@PLT\n";

        // push
        } else if constexpr (std::is_same_v<T, assem::Push>) {
            os << "\tpush\t";
            emit_operand(arg.operand, 8);
            os << "\n";

        // allocate stack
        } else if constexpr (std::is_same_v<T, assem::Allocate_stack>) {
    #ifdef DEBUG_MODE
            os << "# allocate stack space\n";
    #endif
            os << "\tsubq\t$" << arg.size << ", %rsp\n";
    #ifdef DEBUG_MODE
            os << "\n";
    #endif

        // deallocate stack
        } else if constexpr (std::is_same_v<T, assem::Deallocate_stack>) {
#ifdef DEBUG_MODE
            os << "# deallocate stack space\n";
#endif
            os << "\taddq\t$" << arg.size << ", %rsp\n";
#ifdef DEBUG_MODE
            os << "\n";
#endif

        } else {
            assert(false && "internal error in emit_instruction");
        }
    }, i);
}
