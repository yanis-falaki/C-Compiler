#pragma once
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>
#include <unordered_map>


namespace compiler::codegen {

// ------------------------------> Replace PseudoRegisters (1st Pass) <------------------------------

struct ReplacePseudoRegisters {

    std::unordered_map<std::string, uint32_t> mMap;
    uint32_t mLastStackLocation = 0;
    
    // Operand visitors
    ast::asmb::Operand operator()(const ast::asmb::Imm& imm) const {
        return imm;
    }
    
    ast::asmb::Operand operator()(const ast::asmb::Reg& reg) const {
        return reg;
    }

    ast::asmb::Operand operator()(const ast::asmb::Pseudo& pseudo) {
        if (mMap.contains(pseudo.mName))
            return ast::asmb::Stack(mMap[pseudo.mName]);
        // else
        mLastStackLocation -= 4;
        mMap.emplace(pseudo.mName, mLastStackLocation);
        return ast::asmb::Stack(mLastStackLocation);
    }

    ast::asmb::Operand operator()(const ast::asmb::Stack& stack) const {
        return stack;
    }
    
    // Instruction visitors
    void operator()(const ast::asmb::Ret& ret) const {}
    
    void operator()(ast::asmb::Mov& mov) {
        mov.mSrc = std::visit(*this, mov.mSrc);
        mov.mDst = std::visit(*this, mov.mDst);
    }

    void operator()(ast::asmb::Unary& unary) {
        unary.mOperand = std::visit(*this, unary.mOperand);
    }

    void operator()(const ast::asmb::AllocateStack& allocateStack) const {
    }

    // Function visitor
    void operator()(ast::asmb::Function& func) {
        for (auto& instruction : func.mInstructions) {
            std::visit(*this, instruction);
        }
    }

    // Program visitor
    void operator()(ast::asmb::Program& program) {
        (*this)(program.mFunction);
    }
};

// ------------------------------> Code Emission <------------------------------
// A lot of copying, but I'm okay with it for now as it makes it more readable.

struct EmitAsmbVisitor {

    // Operand visitors
    std::string operator() (const ast::asmb::Imm& imm) const {
        return std::format("${}", imm.mValue);
    }

    std::string operator() (const ast::asmb::Reg& reg) const {
        return std::format("\%{}", ast::asmb::reg_name_to_string(reg.mReg));
    }

    std::string operator() (const ast::asmb::Pseudo& pseudo) const {
        // should not have any pseudo registers.
        throw std::runtime_error("Pseudo operand in tree during EmitAsmbVisitor");
    }

    std::string operator() (const ast::asmb::Stack& stack) const {
        // Not yet implemented
        throw std::runtime_error("Stack operand not implemented in EmitAsmbVisitor");
    }

    // Instruction visitors
    std::string operator() (const ast::asmb::Mov& mov) const {
        return std::format("movl {}, {}", std::visit(*this, mov.mSrc), std::visit(*this, mov.mDst));
    }

    std::string operator() (const ast::asmb::Ret ret) const {
        return "ret";
    }

    std::string operator() (const ast::asmb::Unary unary) const {
        // Not yet implemented
        throw std::runtime_error("allocateStack instruction not implemented in EmitAsmbVisitor");
    }

    std::string operator() (const ast::asmb::AllocateStack allocateStack) const {
        // Not yet implemented
        throw std::runtime_error("allocateStack instruction not implemented in EmitAsmbVisitor");
    }

    // Function visitor
    std::string operator()(const ast::asmb::Function& function) {
        std::stringstream ss;
        ss << ".globl " << function.mIdentifier.value() << std::endl;
        ss << function.mIdentifier.value() << ":\n";
        
        for (auto& instruction : function.mInstructions) {
            ss << "\t" << std::visit(*this, instruction) << "\n";
        }
    
        return ss.str();
    }

    // Program
    std::string operator()(const ast::asmb::Program& program) {
        return std::format("{}\n.section .note.GNU-stack,\"\",@progbits\n", (*this)(program.mFunction));
    }
};

}