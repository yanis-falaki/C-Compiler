#include "c_ast.hpp"
#include "asmb_ast.hpp"
#include <memory>
#include <sstream>
#include <format>

namespace compiler::codegen {

// ------------------------------> Conversion from C AST to assembly AST <------------------------------

struct convertVisitor {
    // Expression visitors
    ast::asmb::Imm operator() (const ast::c::Constant& constant) const {
        return ast::asmb::Imm(constant.mValue);
    }

    // Statement visitors
    std::vector<ast::asmb::Instruction> operator() (const ast::c::Return& returnNode) const {
        std::vector<ast::asmb::Instruction> instructions(2);
        instructions[0] = ast::asmb::Mov(std::visit(*this, returnNode.mExpr), ast::asmb::Register("eax"));
        instructions[1] = ast::asmb::Ret();
        return instructions;
    }

    // Not yet implemented
    std::vector<ast::asmb::Instruction> operator() (const ast::c::If& returnNode) const {
        std::vector<ast::asmb::Instruction> instructions;
        return instructions;
    }
};

inline ast::asmb::Function convertFunction(const ast::c::Function& functionNode) {
    return ast::asmb::Function(functionNode.mIdentifier, std::visit(convertVisitor{}, functionNode.mBody));
}

inline ast::asmb::Program convertProgram(const ast::c::Program& program) {
    return ast::asmb::Program(convertFunction(program.mFunction));
}

// ------------------------------> Code Emission <------------------------------
// A lot of copying, but I'm okay with it for now as it makes it more readable.

struct emitAsmbVisitor {
    // operator visitors
    std::string operator() (const ast::asmb::Imm& imm) const {
        return std::format("${}", imm.mValue);
    }

    std::string operator() (const ast::asmb::Register& reg) const {
        return std::format("\%{}", reg.mName);
    }

    // Instruction visitors
    std::string operator() (const ast::asmb::Mov& mov) const {
        return std::format("mov {}, {}", std::visit(*this, mov.mSrc), std::visit(*this, mov.mDst));
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
        ss << "\t" << std::visit(emitAsmbVisitor{}, instruction) << "\n";
    }

    return ss.str();
}

inline std::string emitAsmbProgram(const ast::asmb::Program& program) {
    return std::format("{}\n.section .note.GNU-stack,\"\",@progbits\n", emitAsmbFunction(program.mFunction));
}

}