#pragma once
#include "../ast/ast_c.hpp"
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>

namespace compiler::codegen {

// ------------------------------> Conversion from C AST to assembly AST <------------------------------

struct convertFromCToAsmb {
    // Expression visitors
    ast::asmb::Imm operator() (const ast::c::Constant& constant) const {
        return ast::asmb::Imm(constant.mValue);
    }

    // Expression - Unary Ops, not yet implemented
    ast::asmb::Imm operator() (const ast::c::UnaryOperator& unop) const {
        return std::visit(*this, unop);
    }

    ast::asmb::Imm operator() (const ast::c::BitwiseComplement& complement) const {
        return ast::asmb::Imm(0);
    }

    ast::asmb::Imm operator() (const ast::c::Negation& negation) const {
        return ast::asmb::Imm(0);
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
    return ast::asmb::Function(functionNode.mIdentifier, std::visit(convertFromCToAsmb{}, functionNode.mBody));
}

inline ast::asmb::Program convertProgram(const ast::c::Program& program) {
    return ast::asmb::Program(convertFunction(program.mFunction));
}

}