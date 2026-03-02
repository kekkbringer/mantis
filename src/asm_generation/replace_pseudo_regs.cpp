//
// Created by dominik on 7/22/25.
//

#include <cassert>
#include <variant>
#include <unordered_map>

#include "asm_generator.hpp"
#include "util.hpp"

/**
 * If the operand is a pseudo register, the function checks if it is already in the map of known pseudo registers. If it
 * is, the pseudo register will be replaced with its corresponding stack address. Otherwise, a new address on the stack
 * will be dedicated to the pseudo register, this pair will be saved in the map and the top of the stack will be
 * adjusted accordingly.
 * @param op operand to be replaced
 * @param map map of pseudo registers and corresponding stack addresses
 * @param stack_top keeps track of where the top of the stack currently is.
 */
void Asm_generator::preg_to_stack(assem::Operand& op, std::unordered_map<std::string, int>& map, int& stack_top) {
    // only take action if the operand actually is a pseudo register
    std::visit([&]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, assem::Pseudo>) {
            // try to find pseudo register in map
            // found in map -> just replace
            if (auto search = map.find(arg.name.data()); search != map.end()) {
                op = assem::Stack(search->second);

            // not found in map -> loop up in symbol table
            // if not static -> add, replace and increase stack size
            // if static -> translate to 'Data'
            } else {
                // look up in symbol table
                const Symbol* sym = file_scope->find_global(demangle(arg.name.data()), arg.name.data());
                //if (sym != nullptr and (sym->linkage == Symbol::Linkage::Internal or sym->linkage == Symbol::Linkage::External)) {
                if (sym != nullptr and sym->storage_duration == Symbol::Storage_duration::Static) {
                    op = assem::Data(arg.name.data());

                } else {
                    stack_top -= 4;
                    map[arg.name.data()] = stack_top;
                    op = assem::Stack(stack_top);
                }
            }
        }
    }, op);
}

/**
 * This function looks at each operand of an assembly instruction, checking if it's a pseudo register. If it is, it will
 * replace the pseudo register with an address on the stack. If the pseudo register was already mapped to an address on
 * the stack, the corresponding address will be used. Otherwise, a new address will be allocated on the stack and mapped
 * to that pseudo register. When putting another value onto the stack, the function will keep track of where the new top
 * of the stack is, so that it knows where to put new values and to allocate the needed stack space in the function
 * prolog.
 * @param inst assembly instruction where pseudo register should be replaced
 * @param map map to uniquely connect a stack address to each pseudo register
 * @param stack_top tracks the top of the stack
 */
void Asm_generator::pseudo_reg_inst(assem::Instruction& inst, std::unordered_map<std::string, int>& map, int& stack_top) {
    std::visit([&]<class T0>(T0&& arg){
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, assem::Mov>) {
            preg_to_stack(arg.src, map, stack_top);
            preg_to_stack(arg.dst, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::Unary>) {
            preg_to_stack(arg.operand, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::Binary>) {
            preg_to_stack(arg.src, map, stack_top);
            preg_to_stack(arg.dst, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::Idiv>) {
            preg_to_stack(arg.operand, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::Cmp>) {
            preg_to_stack(arg.lhs, map, stack_top);
            preg_to_stack(arg.rhs, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::SetCC>) {
            preg_to_stack(arg.operand, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::Push>) {
            preg_to_stack(arg.operand, map, stack_top);
        } else if constexpr (std::is_same_v<T, assem::Allocate_stack>) {
        } else if constexpr (std::is_same_v<T, assem::Deallocate_stack>) {
        } else if constexpr (std::is_same_v<T, assem::Ret>) {
        } else if constexpr (std::is_same_v<T, assem::Cdq>) {
        } else if constexpr (std::is_same_v<T, assem::JmpCC>) {
        } else if constexpr (std::is_same_v<T, assem::Jmp>) {
        } else if constexpr (std::is_same_v<T, assem::Label>) {
        } else if constexpr (std::is_same_v<T, assem::Call>) {
        } else {
            assert(false && "in preg_to_stack");
        }
    }, inst);
}

/**
 * This function traverses an assembly program and replaces each pseudo register with a unique address on the stack.
 * It will loop over all top level constructs in the tree, ignoring static variables. For each function definition,
 * it will first fix all instructions within its body and second, set the required stack size in the 'Allocate_stack'
 * instruction at the beginning of the function body.
 * @param prog program where pseudo register should be replaced
 */
void Asm_generator::replace_pseudo_regs(assem::Program& prog) {
    // loop over all top level definitions, only function definitions need to be handled
    for (auto& tp: prog) {
        std::visit([&]<class T0>(T0&& arg){
            using T = std::decay_t<T0>;

            // function found, now loop over all instructions and replace pseudo registers
            if constexpr (std::is_same_v<T, assem::Function>) {
                // keep track of how much is already on the stack
                int stack_top = 0;
                // map pseudo register to stack addresses
                std::unordered_map<std::string, int> pseudo_reg_map;
                for (auto& inst: arg.insts) pseudo_reg_inst(inst, pseudo_reg_map, stack_top);

                // loop over instructions in function, when the first allocate stack instruction is encountered, set its value
                for (auto& i: arg.insts) {
                    if (std::holds_alternative<assem::Allocate_stack>(i)) {
                        // pad stack to be 16 bit aligned
                        const int stack_padding = 16 - (-stack_top)%16;
                        std::get<assem::Allocate_stack>(i).size = -stack_top + stack_padding;
                        break;
                    }
                }
            } else if constexpr (std::is_same_v<T, assem::Static_variable>) {
                // do nothing?!
            }

        }, tp);
    }
}
