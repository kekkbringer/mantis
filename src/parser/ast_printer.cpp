//
// Created by dominik on 7/7/25.
//

#include "parser.hpp"

#include "ast.hpp"

#include <iostream>

void print_declaration(const ast::Declaration_ptr& decl, int& shift);

void print_expression(const ast::Expression_ptr& expr, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << pre << "   monostate\n";
        } else if constexpr (std::is_same_v<T, ast::Constant_ptr>) {
            std::cout << pre << "   constant with value " << arg->val << "\n";;
        } else if constexpr (std::is_same_v<T, ast::Variable_ptr>) {
            std::cout << pre << "   variable: " << arg->name << "\n";
        } else if constexpr (std::is_same_v<T, ast::Unary_ptr>) {
            std::cout << pre << "   Unary " << ast::to_string(arg->op) << "\n";
            print_expression(arg->expr, shift);
        } else if constexpr (std::is_same_v<T, ast::Binary_ptr>) {
            std::cout << pre << "   Binary " << ast::to_string(arg->op) << "\n";
            print_expression(arg->lhs, shift);
            print_expression(arg->rhs, shift);
        } else if constexpr (std::is_same_v<T, ast::Assignment_ptr>) {
            std::cout << pre << "   Assignment\n";
            print_expression(arg->lhs, shift);
            std::cout << pre << "      to\n";
            print_expression(arg->rhs, shift);
        } else if constexpr (std::is_same_v<T, ast::Conditional_ptr>) {
            std::cout << pre << "   Conditional\n";
            std::cout << pre << "      condition:\n";
            print_expression(arg->condition, shift);
            std::cout << pre << "      lhs:\n";
            print_expression(arg->lhs, shift);
            std::cout << pre << "      rhs:\n";
            print_expression(arg->rhs, shift);
        } else if constexpr (std::is_same_v<T, ast::Function_call_ptr>) {
            std::cout << pre << "   Function call\n";
            std::cout << pre << "   name: " << arg->name << "\n";
            std::cout << pre << "   args:\n";
            for (const auto& exp: arg->args) print_expression(exp, shift);
        } else if constexpr (std::is_same_v<T, ast::Post_increment_ptr>) {
            std::cout << pre << "   Post increment\n";
            print_expression(arg->expr, shift);
        } else if constexpr (std::is_same_v<T, ast::Post_decrement_ptr>) {
            std::cout << pre << "   Post decrement\n";
            print_expression(arg->expr, shift);
        } else if constexpr (std::is_same_v<T, ast::Pre_increment_ptr>) {
            std::cout << pre << "   Pre increment\n";
            print_expression(arg->expr, shift);
        } else if constexpr (std::is_same_v<T, ast::Pre_decrement_ptr>) {
            std::cout << pre << "   Pre decrement\n";
            print_expression(arg->expr, shift);
        } else {
            assert(false && "in print expression");
        }
    }, expr);

    shift--;
}

void print_block_item(const ast::Block_item_ptr& bitem, int& shift);

void print_statement(const ast::Statement_ptr& stmt, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << pre << "   monostate\n";
        } else if constexpr (std::is_same_v<T, ast::Return_ptr>) {
            std::cout << pre << "   return\n";
            print_expression(arg->expr, shift);
        } else if constexpr (std::is_same_v<T, ast::If_ptr>) {
            std::cout << pre << "   if\n";
            std::cout << pre << "   condition:\n";
            print_expression(arg->condition, shift);
            std::cout << pre << "      then\n";
            print_statement(arg->then, shift);
            if (not std::holds_alternative<std::monostate>(arg->else_)) {
                std::cout << pre << "      else\n";
                print_statement(arg->else_, shift);
            }
        } else if constexpr (std::is_same_v<T, ast::Labeled_ptr>) {
            std::cout << pre << "   labeled with label: " << arg->label << "\n";
            print_statement(arg->stmt, shift);
        } else if constexpr (std::is_same_v<T, ast::Goto_ptr>) {
            std::cout << pre << "   goto: " << arg->label << "\n";
        } else if constexpr (std::is_same_v<T, ast::Compound_ptr>) {
            std::cout << pre << "   compound\n";
            for (const auto& bitem: *arg->block) {
                print_block_item(bitem, shift);
            }
        } else if constexpr (std::is_same_v<T, ast::Break_ptr>) {
            std::cout << pre << "   break\n";
        } else if constexpr (std::is_same_v<T, ast::Continue_ptr>) {
            std::cout << pre << "   continue\n";
        } else if constexpr (std::is_same_v<T, ast::While_ptr>) {
            std::cout << pre << "   while\n";
            std::cout << pre << "   tagged with: " << arg->tag << "\n";
            std::cout << pre << "   condition:\n";
            print_expression(arg->condition, shift);
            std::cout << pre << "   body:\n";
            print_statement(arg->body, shift);
        } else if constexpr (std::is_same_v<T, ast::Do_while_ptr>) {
            std::cout << pre << "   do while\n";
            std::cout << pre << "   tagged with: " << arg->tag << "\n";
            std::cout << pre << "   condition:\n";
            print_expression(arg->condition, shift);
            std::cout << pre << "   body:\n";
            print_statement(arg->body, shift);
        } else if constexpr (std::is_same_v<T, ast::For_ptr>) {
            std::cout << pre << "   for\n";
            std::cout << pre << "   tagged with " << arg->tag << "\n";
            // for init
            std::visit([&]<class U0>(U0&& arg2) {
                using U = std::decay_t<U0>;
                if constexpr (std::is_same_v<U, ast::Declaration_ptr>) {
                    std::cout << pre << "   init:\n";
                    print_declaration(arg2, shift);
                } else if constexpr (std::is_same_v<U, ast::Expression_ptr>) {
                    std::cout << pre << "   init:\n";
                    print_expression(arg2, shift);
                }
            }, arg->init);
            // condition
            if (not std::holds_alternative<std::monostate>(arg->condition)) {
                std::cout << pre << "   condition:\n";
                print_expression(arg->condition, shift);
            }
            // post
            if (not std::holds_alternative<std::monostate>(arg->post)) {
                std::cout << pre << "   post:\n";
                print_expression(arg->post, shift);
            }
            // body
            std::cout << pre << "   body:\n";
            print_statement(arg->body, shift);
        } else if constexpr (std::is_same_v<T, ast::Null_ptr>) {
            std::cout << pre << "   null\n";
        } else if constexpr (std::is_same_v<T, ast::Expression_ptr>) {
            std::cout << pre << "   expression statement\n";
            print_expression(arg, shift);
        } else if constexpr (std::is_same_v<T, ast::Default_ptr>) {
            std::cout << pre << "   default with tag: " << arg->tag << "\n";
            std::cout << pre << "    stmt:\n";
            print_statement(arg->stmt, shift);
        } else if constexpr (std::is_same_v<T, ast::Case_ptr>) {
            std::cout << pre << "   case with val " << arg->num << " and with tag: " << arg->tag << "\n";
            std::cout << pre << "    stmt:\n";
            print_statement(arg->stmt, shift);
        } else if constexpr (std::is_same_v<T, ast::Switch_ptr>) {
            std::cout << pre << "   switch with tag " << arg->tag << "\n";
            std::cout << pre << "   cases: ";
            for (const auto& i: arg->cases) std::cout << i << " ";
            std::cout << pre << "\n   has default: " << arg->has_default << "\n";
            std::cout << pre << "   body:\n";
            print_statement(arg->body, shift);
        } else {
            assert(false && "in print statement");
        }
    }, stmt);

    shift--;
}

void print_block_item(const ast::Block_item_ptr& bitem, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, ast::Statement_ptr>) {
            print_statement(arg, shift);
        } else if constexpr (std::is_same_v<T, ast::Declaration_ptr>) {
            print_declaration(arg, shift);
        }
    }, bitem);


    shift--;
}

void print_block(const ast::Block_ptr& block, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";
    std::cout << pre << "     BLOCK\n";
    if (block != nullptr) for (const auto& item : *block) {print_block_item(item, shift);}

    shift--;
}

void print_declaration(const ast::Declaration_ptr& decl, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    std::visit([&]<class T0>(T0&& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, ast::Function_declaration_ptr>) {
            std::cout << pre << "     FUNCTION_DECLARATION\n";
            std::cout << pre << "     name: " << arg->name << "\n";
            std::cout << pre << "     params: ";
            for (const auto& p : arg->params) {std::cout << p->name << "   ";}  std::cout << "\n";
            switch (arg->storage_class) {
                case ast::Storage_class::None: break;
                case ast::Storage_class::Static: std::cout << "|    storage class: static\n"; break;
                case ast::Storage_class::Extern: std::cout << "|    storage class: extern\n"; break;
                default: break;
            }
            std::cout << pre << "     body:\n";
            print_block(arg->body, shift);
        } else if constexpr (std::is_same_v<T, ast::Variable_declaration_ptr>) {
            std::cout << pre << "VARIABLE_DECLARATION" << std::endl;
            std::cout << pre << "   name: " << arg->name << "\n";
            switch (arg->storage_class) {
                case ast::Storage_class::None: std::cout << pre << "   storage class: None\n"; break;
                case ast::Storage_class::Static: std::cout << pre << "   storage class: Static\n"; break;
                case ast::Storage_class::Extern: std::cout << pre << "   storage class: Extern\n"; break;
            }
            if (not std::holds_alternative<std::monostate>(arg->init)) {
                std::cout << pre << "   init:\n";
                print_expression(arg->init, shift);
            }
        }
    }, decl);

    shift--;
}

void Parser::print_program(const ast::Program& prog) {
    std::cout << ":: PROGRAM HEAD\n";
    int shift = 0;
    for (const auto& decl : prog.declarations) {
        print_declaration(decl, shift);
    }
}

void Parser::print_AST() {
    print_program(parse());
}
