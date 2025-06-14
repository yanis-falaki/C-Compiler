#pragma once
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>


namespace compiler::code_emission {

// ------------------------------> Replace PseudoRegisters (1st Pass) <------------------------------

// ------------------------------> Code Emission <------------------------------
// A lot of copying, but I'm okay with it for now as it makes it more readable.

struct EmitAsmbVisitor {
    // operator visitors
    std::string operator() (const ast::asmb::Imm& imm) const {
        return std::format("${}", imm.mValue);
    }

    std::string operator() (const ast::asmb::Reg& reg) const {
        return std::format("\%{}", ast::asmb::reg_name_to_string(reg.mReg));
    }

    // Instruction visitors
    std::string operator() (const ast::asmb::Mov& mov) const {
        return std::format("movl {}, {}", std::visit(*this, mov.mSrc), std::visit(*this, mov.mDst));
    }

    std::string operator() (const ast::asmb::Ret ret) const {
        return "ret";
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