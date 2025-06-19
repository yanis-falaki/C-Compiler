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

    void operator()(const Binary& binary) const {
        std::cout << indent() << "Binary: " << binary_op_to_string(binary.mOp) << std::endl;
        std::cout << indent() << "  " << "Operand 1:\n";
        std::visit(PrintVisitor(depth+2), binary.mOperand1);
        std::cout << indent() << "  " << "Operand 2:\n";
        std::visit(PrintVisitor(depth+2), binary.mOperand2);
    }

    void operator()(const Idiv& idiv) const {
        std::cout << indent() << "Idiv:\n";
        std::cout << indent() << "  " << "Operand:\n";
        std::visit(PrintVisitor(depth+2), idiv.mOperand);
    }

    void operator()(const Cdq&) const {
        std::cout << indent() << "Cdq" << std::endl;
    }

    void operator()(const AllocateStack& allocateStack) const {
        std::cout << indent() << "Allocate Stack: " << allocateStack.mValue << std::endl;
    }

    void operator()(const Cmp& cmp) const {
        std::cout << indent() << "Cmp:\n";
        std::cout << indent() << "  Operand 1:\n";
        std::visit(PrintVisitor(depth+2), cmp.mOperand1);
        std::cout << indent() << "  Operand 2:\n";
        std::visit(PrintVisitor(depth+2), cmp.mOperand2);
    }

    void operator()(const Jmp& jmp) const {
        std::cout << indent() << "Jmp: " << jmp.mIdentifier << std::endl;
    }

    void operator()(const JmpCC& jmpCC) const {
        std::cout << indent() << "JumpCC: " << jmpCC.mIdentifier << std::endl;
        std::cout << indent() << "  Condition Code: "
            << condition_code_to_string(jmpCC.mCondCode) << std::endl;
    }

    void operator()(const SetCC& setCC) const {
        std::cout << indent() << "SetCC:\n";
        std::cout << indent() << "  Condition Code: "
            << condition_code_to_string(setCC.mCondCode) << std::endl;
        std::cout << indent() << "  Operand:\n";
        if (std::holds_alternative<ast::asmb::Reg>(setCC.mDst)) {
            std::cout << indent() << "    Reg: " << ast::asmb::reg_name_to_string(
                std::get<ast::asmb::Reg>(setCC.mDst).mReg, 
                ast::asmb::RegisterSize::BYTE) << std::endl;
        }
        else
            std::visit(PrintVisitor(depth+2), setCC.mDst);
        
    }

    void operator()(const Label& label) const {
        std::cout << indent() << "Label: " << label.mIdentifier << std::endl;
    }

    // Function visitor
    void operator()(const Function& func) {
        if (func.mIdentifier.has_value()) {
            std::cout << indent() << "Function " << func.mIdentifier.value() << ":" << std::endl;
        } else {
            std::cout << indent() << "Function:" << std::endl;
        }
        
        for (const auto& instruction : func.mInstructions) {
            std::visit(PrintVisitor{depth + 1}, instruction);
        }
    }

    // Program visitor
    void operator()(const Program& program) {
        (*this)(program.mFunction);
    }
};

}