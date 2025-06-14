#pragma once
#include "../ast/ast_c.hpp"
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>

/*
Deprecated, file is kept for reference.
*/

namespace compiler::codegen {

// ------------------------------> Conversion from C AST to assembly AST <------------------------------

struct ConvertFromCToAsmb {
    // Expression visitors
    ast::asmb::Imm operator() (const ast::c::Constant& constant) const {
        return ast::asmb::Imm(constant.mValue);
    }

    // Expression Unary
    ast::asmb::Imm operator() (const ast::c::Unary& unary) const {
        return std::visit(*this, *unary.mExpr);
    }

    // Statement visitors
    std::vector<ast::asmb::Instruction> operator() (const ast::c::Return& returnNode) const {
        std::vector<ast::asmb::Instruction> instructions(2);
        instructions[0] = ast::asmb::Mov(std::visit(*this, returnNode.mExpr),
                          ast::asmb::Reg(ast::asmb::RegisterName::AX));
        instructions[1] = ast::asmb::Ret();
        return instructions;
    }

    // Not yet implemented
    std::vector<ast::asmb::Instruction> operator() (const ast::c::If& returnNode) const {
        std::vector<ast::asmb::Instruction> instructions;
        return instructions;
    }
};

inline ast::asmb::Function convertCFunctionToAsmb(const ast::c::Function& functionNode) {
    return ast::asmb::Function(functionNode.mIdentifier, std::visit(ConvertFromCToAsmb{}, functionNode.mBody));
}

inline ast::asmb::Program convertCProgramToAsmb(const ast::c::Program& program) {
    return ast::asmb::Program(convertCFunctionToAsmb(program.mFunction));
}

}