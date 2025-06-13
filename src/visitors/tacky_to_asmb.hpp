#pragma once
#include "../ast/ast_tacky.hpp"
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>
#include <vector>


namespace compiler::codegen {

// ------------------------------> Map between TACKY unops and ASMB unops <------------------------------

inline constexpr ast::asmb::UnaryOperator tacky_to_asmb_unop(ast::tacky::UnaryOperator unop) {
    switch (unop) {
        case ast::tacky::UnaryOperator::Negate:          return ast::asmb::UnaryOperator::Negate;
        case ast::tacky::UnaryOperator::Complement:      return ast::asmb::UnaryOperator::Complement;
        default:
            throw std::runtime_error("tacky_to_asmb_unop received an unknown ast::tacky::UnaryOperator");
            break;
    }
}

// ------------------------------> TackyToAsmb <------------------------------

struct TackyToAsmb {

    std::vector<ast::asmb::Instruction> mInstructions;
    
    // Val visitors
    ast::asmb::Operand operator()(const ast::tacky::Constant& constant) const {
        return ast::asmb::Imm(constant.mValue);
    }

    ast::asmb::Operand operator()(const ast::tacky::Var& var) const {
        return ast::asmb::Pseudo(var.mIdentifier);
    }

    // Instruction visitors
    void operator()(const ast::tacky::Return& ret) {
        ast::asmb::Operand src = std::visit(*this, ret.mVal);
        mInstructions.emplace_back(ast::asmb::Mov(std::move(src), ast::asmb::Reg(ast::asmb::RegisterName::AX)));
        mInstructions.emplace_back(ast::asmb::Ret());
    }

    void operator()(const ast::tacky::Unary& unary) {
        ast::asmb::Operand src = std::visit(*this, unary.mSrc);
        ast::asmb::Operand dst = std::visit(*this, unary.mDst);
        ast::asmb::UnaryOperator unop = tacky_to_asmb_unop(unary.mOp);
        mInstructions.emplace_back(ast::asmb::Mov(std::move(src), dst));
        mInstructions.emplace_back(ast::asmb::Unary(unop, dst));
    }
};

inline ast::asmb::Function tackyFunctionToAsmb(const ast::tacky::Function& func) {
    TackyToAsmb converter;
    for (auto& instruction : func.mBody) {
        std::visit(converter, instruction);
    }
    return ast::asmb::Function(func.mIdentifier, std::move(converter.mInstructions));
}

inline ast::asmb::Program tackyProgramToAsmb(const ast::tacky::Program& program) {
    return ast::asmb::Program(tackyFunctionToAsmb(program.mFunction));
}
    
}