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

    void operator()(const Binary& binary) const {
        std::cout << indent() << "Binary: " << binary_op_to_string(binary.mOp) << std::endl;
        std::cout << indent() << "  " << "Source 1:\n";
        std::visit(PrintVisitor(depth+2), binary.mSrc1);
        std::cout << indent() << "  " << "Source 2:\n";
        std::visit(PrintVisitor(depth+2), binary.mSrc2);
        std::cout << indent() << "  " << "Destination:\n";
        std::visit(PrintVisitor(depth+2), binary.mDst); 
    }

    void operator()(const Copy& copy) const {
        std::cout << indent() << "Copy:\n";
        std::cout << indent() << "  " << "Source:\n";
        std::visit(PrintVisitor(depth+2), copy.mSrc);
        std::cout << indent() << "  " << "Destination:\n";
        std::visit(PrintVisitor(depth+2), copy.mDst);
    }

    void operator()(const Jump& jump) const {
        std::cout << indent() << "Jump: " << jump.mTarget << std::endl;
    }

    void operator()(const JumpIfZero& jumpIfZero) const {
        std::cout << indent() << "Jump If Zero: " << jumpIfZero.mTarget << std::endl;
        std::cout << indent() << "  " << "Condition:\n";
        std::visit(PrintVisitor(depth+2), jumpIfZero.mCondition);
    }

    void operator()(const JumpIfNotZero& jumpIfNotZero) const {
        std::cout << indent() << "Jump If Not Zero: " << jumpIfNotZero.mTarget << std::endl;
        std::cout << indent() << "  " << "Condition:\n";
        std::visit(PrintVisitor(depth+2), jumpIfNotZero.mCondition);
    }

    void operator()(const Label& label) const {
        std::cout << indent() << "Label: " << label.mIdentifier << std::endl;
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