#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include "../ast/ast_tacky.hpp"

namespace compiler::ast::tacky {

// ------------------------------> Printing Utils <------------------------------

constexpr std::string_view unary_op_to_string(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement: return "Complement";
        case UnaryOperator::Negate:     return "Negate";
    }
    return "Unknown"; // Optional, unreachable with exhaustive handling
}

// Print Visitor
struct PrintVisitor {
    uint32_t depth;
    
    explicit PrintVisitor(uint32_t d = 0) : depth(d) {}
    
    std::string indent() const {
        return std::string(depth * 2, ' ');
    }
    
    // Val visitors
    void operator()(const Val& val) const {
        std::visit(*this, val);
    }

    void operator()(const Constant& constant) const {
        std::cout << indent() << "Constant: " << constant.mValue << std::endl;
    }

    void operator()(const Var& var) const {
        std::cout << indent() << "Var: " << var.mIdentifier << std::endl;
    }

    // Instruction visitors
    void operator()(const Instruction& instruction) const {
        std::visit(*this, instruction);
    }

    void operator()(const Return& ret) const {
        std::cout << indent() << "Return:\n";
        std::visit(*this, ret.mVal);
    }

    void operator()(const Unary& unary) const {
        std::cout << indent() << "Unary: " << unary_op_to_string(unary.mOp) << std::endl;
        std::cout << indent() << "  " << "Source:\n";
        std::visit(PrintVisitor(depth+2), unary.mSrc);
        std::cout << indent() << "  " << "Destination:\n";
        std::visit(PrintVisitor(depth+2), unary.mDst);
    }
};

inline void printAST(const Function& func, uint32_t depth = 0) {
    std::string indent = std::string(depth * 2, ' ');
    std::cout << indent << "Function:" << std::endl;
    for (const auto& instruction : func.mBody) {
        std::visit(PrintVisitor{depth+1}, instruction);
    }
}

inline void printAST(const Program& program, uint32_t depth = 0) {
    printAST(program.mFunction, depth);
}

}