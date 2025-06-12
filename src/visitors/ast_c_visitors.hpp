#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include "../ast/ast_c.hpp"

namespace compiler::ast::c {

// ------------------------------> Printing Utils <------------------------------

// Print Visitor
struct PrintVisitor {
    uint32_t depth;
    
    explicit PrintVisitor(uint32_t d = 0) : depth(d) {}
    
    std::string indent() const {
        return std::string(depth * 2, ' ');
    }
    
    // Expression visitors
    void operator()(const Constant& constant) const {
        std::cout << indent() << "Constant: " << constant.mValue << std::endl;
    }

    // Expressions - Unary Operators
    void operator()(const UnaryOperator& unop) const {
        std::visit(*this, unop);
    }

    void operator()(const BitwiseComplement& complement) const {
        std::cout << indent() << "Bitwise Complement:"<< std::endl;
        std::visit(PrintVisitor(depth+1), *(complement.mExpr));
    }

    void operator()(const Negation& negation) const {
        std::cout << indent() << "Negation:"<< std::endl;
        std::visit(PrintVisitor(depth+1), *(negation.mExpr));
    }
    
    // Statement visitors
    void operator()(const Return& ret) const {
        std::cout << indent() << "Return:" << std::endl;
        std::visit(PrintVisitor(depth + 1), ret.mExpr);
    }
    
    void operator()(const If& ifStmt) const {
        std::cout << indent() << "If:" << std::endl;
        std::cout << indent() << "  Condition:" << std::endl;
        std::visit(PrintVisitor(depth + 2), ifStmt.mCondition);
        std::cout << indent() << "  Then:" << std::endl;
        std::visit(PrintVisitor(depth + 2), *ifStmt.mThen);
        if (ifStmt.mElse.has_value()) {
            std::cout << indent() << "  Else:" << std::endl;
            std::visit(PrintVisitor(depth + 2), *ifStmt.mElse.value());
        }
    }
};

inline void printAST(const Function& func, uint32_t depth = 0) {
    std::string indent = std::string(depth * 2, ' ');
    if (func.mIdentifier.has_value()) {
        std::cout << indent << "Function " << func.mIdentifier.value() << ":" << std::endl;
    } else {
        std::cout << indent << "Function:" << std::endl;
    }
    std::visit(PrintVisitor(depth+1), func.mBody);
}

inline void printAST(const Program& program, uint32_t depth = 0) {
    printAST(program.mFunction, depth);
}

}