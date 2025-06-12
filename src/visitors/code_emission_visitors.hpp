#pragma once
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>


namespace compiler::code_emission {

// ------------------------------> Code Emission <------------------------------
// A lot of copying, but I'm okay with it for now as it makes it more readable.

struct EmitAsmbVisitor {
    // operator visitors
    std::string operator() (const ast::asmb::Imm& imm) const {
        return std::format("${}", imm.mValue);
    }

    std::string operator() (const ast::asmb::Register& reg) const {
        return std::format("\%{}", reg.mName);
    }

    // Instruction visitors
    std::string operator() (const ast::asmb::Mov& mov) const {
        return std::format("movl {}, {}", std::visit(*this, mov.mSrc), std::visit(*this, mov.mDst));
    }

    std::string operator() (const ast::asmb::Ret ret) const {
        return "ret";
    }
};

inline std::string emitAsmbFunction(const ast::asmb::Function& function) {
    std::stringstream ss;
    ss << ".globl " << function.mIdentifier.value() << std::endl;
    ss << function.mIdentifier.value() << ":\n";
    
    for (auto& instruction : function.mInstructions) {
        ss << "\t" << std::visit(EmitAsmbVisitor{}, instruction) << "\n";
    }

    return ss.str();
}

inline std::string emitAsmbProgram(const ast::asmb::Program& program) {
    return std::format("{}\n.section .note.GNU-stack,\"\",@progbits\n", emitAsmbFunction(program.mFunction));
}

}