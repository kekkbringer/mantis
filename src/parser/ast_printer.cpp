//
// Created by dominik on 7/7/25.
//

#include "parser.hpp"
#include "ast.hpp"
#include "util.hpp"

#include <iostream>

void print_block(const ast::Block* block, int& shift);
void print_declaration(const ast::Decl* decl, int& shift);

void print_expression(const ast::Expr* expr, int& shift) {
    if (expr == nullptr) return;
       
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    switch (expr->kind) {
        case ast::Expr::Kind::Constant: {
            const auto* con = cast<ast::Constant const>(expr);
            std::cout << pre << "-constant with value " << con->val << "\n";;
            break;
        }

        case ast::Expr::Kind::Variable: {
            const auto* var = cast<ast::Variable const>(expr);
            std::cout << pre << "VARIABLE: " << var->name << "\n";
            break;
        }

        case ast::Expr::Kind::Unary: {
            const auto* un = cast<ast::Unary const>(expr);
            std::cout << pre << "UNARY " << ast::to_string(un->op) << "\n";
            print_expression(un->operand, shift);
            break;
        }

        case ast::Expr::Kind::Binary: {
            const auto* bin = cast<ast::Binary const>(expr);
            std::cout << pre << "BINARY " << ast::to_string(bin->op) << "\n";
            print_expression(bin->lhs, shift);
            print_expression(bin->rhs, shift);
            break;
        }

        case ast::Expr::Kind::Assignment: {
            const auto* ass = cast<ast::Assignment const>(expr);
            std::cout << pre << "ASSIGNMENT\n";
            std::cout << pre << "-to\n";
            print_expression(ass->lhs, shift);
            std::cout << pre << "-from\n";
            print_expression(ass->rhs, shift);
            break;
        }

        case ast::Expr::Kind::Conditional: {
            const auto* con = cast<ast::Conditional const>(expr);
            std::cout << pre << "CONDITIONAL\n";
            std::cout << pre << "-condition:\n";
            print_expression(con->cond, shift);
            std::cout << pre << "-then:\n";
            print_expression(con->then, shift);
            std::cout << pre << "-else:\n";
            print_expression(con->else_, shift);
            break;
        }

        case ast::Expr::Kind::Function_call: {
            const auto* fun_call = cast<ast::Function_call const>(expr);
            std::cout << pre << "FUNCTION CALL\n";
            std::cout << pre << "-name: " << fun_call->name << "\n";
            std::cout << pre << "-args:\n";
            for (const auto& exp: fun_call->arguments()) print_expression(exp, shift);
            break;
        }

        case ast::Expr::Kind::Inc_dec: {
            const auto* incdec = cast<ast::Inc_dec const>(expr);
            if (incdec->is_prefix) std::cout << pre << "PREFIX ";
            else std::cout << pre << "SUFFIX ";
            if (incdec->is_increment) std::cout << "INCREMENT\n";
            else std::cout << "DECREMENT\n";
            print_expression(incdec->operand, shift);
            break;
        }

        case ast::Expr::Kind::Error: {
            std::cout << pre << "ERROR EXPR\n";
            break;
        }
    }

    shift--;
}

void print_block_item(const ast::Block_item* bitem, int& shift);

void print_statement(const ast::Stmt* stmt, int& shift) {
    if (stmt == nullptr) return;

    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    switch (stmt->kind) {
        case ast::Stmt::Kind::Return: {
            const auto* ret = cast<ast::Return const>(stmt);
            std::cout << pre << "RETURN STMT\n";
            std::cout << pre << "-expr:\n";
            print_expression(ret->expr, shift);
            break;
        }

        case ast::Stmt::Kind::If: {
            const auto* if_ = cast<ast::If const>(stmt);
            std::cout << pre << "IF STMT\n";
            std::cout << pre << "-condition:\n";
            print_expression(if_->condition, shift);
            std::cout << pre << "-then:\n";
            print_statement(if_->then, shift);
            if (if_->else_ != nullptr) {
                std::cout << pre << "-else:\n";
                print_statement(if_->else_, shift);
            }
            break;
        }

        case ast::Stmt::Kind::Compound: {
            const auto* com = cast<ast::Compound const>(stmt);
            std::cout << pre << "COMPOUND STMT\n";
            print_block(com->block, shift);
            break;
        }

        case ast::Stmt::Kind::Break: {
            const auto* break_ = cast<ast::Break const>(stmt);
            std::cout << pre << "BREAK STMT\n";
            std::cout << pre << "-tag: " << break_->tag << "\n";
            break;
        }

        case ast::Stmt::Kind::Continue: {
            const auto* con = cast<ast::Continue const>(stmt);
            std::cout << pre << "CONTINUE STMT\n";
            std::cout << pre << "-tag: " << con->tag << "\n";
            break;
        }

        case ast::Stmt::Kind::While: {
            const auto* while_ = cast<ast::While const>(stmt);
            std::cout << pre << "WHILE STMT\n";
            std::cout << pre << "-tag: " << while_->tag << "\n";
            std::cout << pre << "-condition:\n";
            print_expression(while_->condition, shift);
            std::cout << pre << "-body:\n";
            print_statement(while_->body, shift);
            break;
        }

        case ast::Stmt::Kind::Do_while: {
            const auto* dwloop = cast<ast::Do_while const>(stmt);
            std::cout << pre << "DO WHILE STMT\n";
            std::cout << pre << "-tag: " << dwloop->tag << "\n";
            std::cout << pre << "-condition:\n";
            print_expression(dwloop->condition, shift);
            std::cout << pre << "-body:\n";
            print_statement(dwloop->body, shift);
            break;
        }

        case ast::Stmt::Kind::For: {
            const auto* forloop = cast<ast::For const>(stmt);
            std::cout << pre << "FOR STMT\n";
            std::cout << pre << "-tag: " << forloop->tag << "\n";
            if (forloop->init.kind == ast::For_init::Kind::None) {
            } else if (forloop->init.kind == ast::For_init::Kind::Decl) {
                std::cout << pre << "-init:\n";
                print_declaration(forloop->init.var_decl, shift);
            } else if (forloop->init.kind == ast::For_init::Kind::Expr) {
                std::cout << pre << "-init:\n";
                print_expression(forloop->init.expr, shift);
            }
            // condition
            if (forloop->condition != nullptr) {
                std::cout << pre << "-condition:\n";
                print_expression(forloop->condition, shift);
            }
            // post
            if (forloop->post != nullptr) {
                std::cout << pre << "-post:\n";
                print_expression(forloop->post, shift);
            }
            // body
            std::cout << pre << "-body:\n";
            print_statement(forloop->body, shift);
            break;
        }

        case ast::Stmt::Kind::Null: {
            std::cout << pre << "NULL STMT\n";
            break;
        }

        case ast::Stmt::Kind::Labeled: {
            const auto* lab = cast<ast::Labeled const>(stmt);
            std::cout << pre << "LABELED STMT with label: " << lab->label << "\n";
            print_statement(lab->stmt, shift);
            break;
        }

        case ast::Stmt::Kind::Goto: {
            const auto* goto_ = cast<ast::Goto const>(stmt);
            std::cout << pre << "GOTO STMT to " << goto_->label << "\n";
            break;
        }

        case ast::Stmt::Kind::Switch: {
            const auto* switch_ = cast<ast::Switch const>(stmt);
            std::cout << pre << "SWITCH STMT with tag " << switch_->tag << "\n";
            std::cout << pre << "-cases: ";
            for (const auto& i: switch_->case_nums()) std::cout << i << ", ";
            std::cout << "\n";
            std::cout << pre << "-has default: " << switch_->has_default << "\n";
            std::cout << pre << "   body:\n";
            print_statement(switch_->body, shift);
            break;
        }

        case ast::Stmt::Kind::Switch_label: {
            const auto* slab = cast<ast::Switch_label const>(stmt);
            if (slab->is_default) {
                std::cout << pre << "DEFAULT with tag: " << slab->tag << "\n";
            } else {
                std::cout << pre << "CASE with val " << slab->value << " and with tag: " << slab->tag << "\n";
            }
            std::cout << pre << "-stmt:\n";
            print_statement(slab->stmt, shift);
            break;
        }

        case ast::Stmt::Kind::Expr: {
            const auto* expr = cast<ast::Expr_stmt const>(stmt);
            std::cout << pre << "EXPRESSION STMT\n";
            std::cout << pre << "-expr:\n";
            print_expression(expr->expr, shift);
            break;
        }

        case ast::Stmt::Kind::Error: {
            std::cout << pre << "ERROR STMT\n";
            break;
        }
    }

    shift--;
}

void print_block_item(const ast::Block_item& bitem, int& shift) {
    if (bitem.kind == ast::Block_item::Kind::Stmt)
        print_statement(bitem.stmt, shift);
    else if (bitem.kind == ast::Block_item::Kind::Decl)
        print_declaration(bitem.decl, shift);
    else
        std::cout << "UNKNOWN BLOCK ITEM\n";
}

void print_block(const ast::Block* block, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";
    std::cout << pre << "BLOCK\n";
    if (block != nullptr)
        for (const auto& item: block->item_span())
            print_block_item(item, shift);

    shift--;
}

void print_declaration(const ast::Decl* decl, int& shift) {
    shift++;
    std::string pre = "";
    for (int i=0; i<shift-1; i++) pre += "   ";

    if (decl->kind == ast::Decl::Kind::Var) {
        const auto* var_ptr = cast<ast::Var_decl const>(decl);
        std::cout << pre << "VARIABLE DECLARATION" << std::endl;
        std::cout << pre << "-name: " << var_ptr->name << "\n";
        switch (decl->storage_class) {
            case ast::Decl::Storage_class::None: std::cout << pre << "-storage class: None\n"; break;
            case ast::Decl::Storage_class::Static: std::cout << pre << "-storage class: Static\n"; break;
            case ast::Decl::Storage_class::Extern: std::cout << pre << "-storage class: Extern\n"; break;
        }
        if (var_ptr->init != nullptr) {
            std::cout << pre << "-init:\n";
            print_expression(var_ptr->init, shift);
        }

    } else if (decl->kind == ast::Decl::Kind::Func) {
        const auto* func_ptr = cast<ast::Func_decl const>(decl);
        std::cout << pre << "FUNCTION DECLARATION" << std::endl;
        std::cout << pre << "-name: " << func_ptr->name << "\n";
        std::cout << pre << "-params: ";
        for (const auto& p : func_ptr->parameters()) {
            std::cout << p->name << ", ";
        }
        std::cout << "\n";
        switch (func_ptr->storage_class) {
            case ast::Decl::Storage_class::None: break;
            case ast::Decl::Storage_class::Static: std::cout << "-storage class: static\n"; break;
            case ast::Decl::Storage_class::Extern: std::cout << "-storage class: extern\n"; break;
            default: break;
        }
        std::cout << pre << "-body:\n";
        print_block(func_ptr->body, shift);

    } else if (decl->kind == ast::Decl::Kind::Error) {
        std::cout << pre << "ERROR DECL\n";

    } else {
        std::cout << "UNKNOWN DECL\n";
    }
    shift--;
}

void Parser::print_program(const ast::Program* prog) {
    std::cout << ":: PROGRAM HEAD\n";
    int shift = 0;
    for (const auto& decl : prog->declarations()) {
        print_declaration(decl, shift);
    }
}

void Parser::print_AST() {
    //print_program(parse());
}
