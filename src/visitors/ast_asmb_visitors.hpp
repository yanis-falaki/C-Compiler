#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include "../ast/ast_asmb.hpp"

namespace compiler::ast::asmb {

// ------------------------------> Printing Utils <------------------------------

// Print Visitor
struct PrintVisitor {
    uint32_t depth;
    
    explicit PrintVisitor(uint32_t d = 0) : depth(d) {}
    
    std::string indent() const {
        return std::string(depth * 2, ' ');
    }
    
    // Operand visitors
    void operator()(const Imm& imm) const {
        std::cout << indent() << "Imm: " << imm.mValue << std::endl;
    }
    
    void operator()(const Reg& reg) const {
        std::cout << indent() << "Reg: " << reg_name_to_string(reg.mReg) << std::endl;
    }

    void operator()(const Pseudo& pseudo) const {
        std::cout << indent() << "Pseudo: " << pseudo.mName << std::endl;
    }

    void operator()(const Stack& stack) const {
        std::cout << indent() << "Stack: " << stack.mLocation << std::endl;
    }
    
    // Instruction visitors
    void operator()(const Ret& ret) const {
        std::cout << indent() << "Ret" << std::endl;
    }
    
    void operator()(const Mov& mov) const {
        std::cout << indent() << "Mov:" << std::endl;
        std::cout << indent() << "  Source:" << std::endl;
        std::visit(PrintVisitor(depth + 2), mov.mSrc);
        std::cout << indent() << "  Destination:" << std::endl;
        std::visit(PrintVisitor(depth + 2), mov.mDst);
    }

    void operator()(const Unary& unary) const {
        std::cout << indent() << "Unary: " << unary_op_to_string(unary.mOp) << std::endl;
        std::cout << indent() << "  " << "Operand:\n";
        std::visit(PrintVisitor(depth+2), unary.mOperand);
    }

    void operator()(const AllocateStack& allocateStack) const {
        std::cout << indent() << "Allocate Stack: " << allocateStack.mValue << std::endl;
    }
};

inline void printAST(const Operand& operand, uint32_t depth = 0) {
    std::visit(PrintVisitor(depth), operand);
}

inline void printAST(const Instruction& instruction, uint32_t depth = 0) {
    std::visit(PrintVisitor(depth), instruction);
}

inline void printAST(const Function& func, uint32_t depth = 0) {
    std::string indent = std::string(depth * 2, ' ');
    if (func.mIdentifier.has_value()) {
        std::cout << indent << "Function " << func.mIdentifier.value() << ":" << std::endl;
    } else {
        std::cout << indent << "Function:" << std::endl;
    }
    
    for (const auto& instruction : func.mInstructions) {
        printAST(instruction, depth + 1);
    }
}

inline void printAST(const Program& program, uint32_t depth = 0) {
    printAST(program.mFunction, depth);
}

}