#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include "../ast/ast_tacky.hpp"

namespace compiler::ast::tacky {

// ------------------------------> Printing Utils <------------------------------

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
        std::visit(PrintVisitor(depth+1), ret.mVal);
    }

    void operator()(const Unary& unary) const {
        std::cout << indent() << "Unary: " << unary_op_to_string(unary.mOp) << std::endl;
        std::cout << indent() << "  " << "Source:\n";
        std::visit(PrintVisitor(depth+2), unary.mSrc);
        std::cout << indent() << "  " << "Destination:\n";
        std::visit(PrintVisitor(depth+2), unary.mDst);
    }

    // Function visitor
    void operator()(const Function& func) {
        std::string indent = std::string(depth * 2, ' ');
        std::cout << indent << "Function " << func.mIdentifier << ":\n";
        for (const auto& instruction : func.mBody) {
            std::visit(PrintVisitor{depth+1}, instruction);
        }
    }

    // Program visitor
    void operator()(const Program& program) {
        (*this)(program.mFunction);
    }
};

}