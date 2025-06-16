#pragma once
#include "../ast/ast_tacky.hpp"
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>
#include <vector>
#include <unordered_map>


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

// ------------------------------> Map between TACKY binops and ASMB unops <------------------------------

inline constexpr ast::asmb::BinaryOperator tacky_to_asmb_binop(ast::tacky::BinaryOperator unop) {
    switch (unop) {
        case ast::tacky::BinaryOperator::Add:             return ast::asmb::BinaryOperator::Add;
        case ast::tacky::BinaryOperator::Subtract:        return ast::asmb::BinaryOperator::Subtract;
        case ast::tacky::BinaryOperator::Multiply:        return ast::asmb::BinaryOperator::Multiply;
        case ast::tacky::BinaryOperator::Left_Shift:      return ast::asmb::BinaryOperator::Left_Shift;
        case ast::tacky::BinaryOperator::Right_Shift:     return ast::asmb::BinaryOperator::Right_Shift;
        case ast::tacky::BinaryOperator::Bitwise_AND:     return ast::asmb::BinaryOperator::Bitwise_AND;
        case ast::tacky::BinaryOperator::Bitwise_OR:      return ast::asmb::BinaryOperator::Bitwise_OR;
        case ast::tacky::BinaryOperator::Bitwise_XOR:     return ast::asmb::BinaryOperator::Bitwise_XOR;
    }
    throw std::runtime_error("tacky_to_asmb_binop received an unknown ast::tacky::BinaryOperator");
}

// ------------------------------> TackyToAsmb (0th pass) <------------------------------

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
        mInstructions.emplace_back(ast::asmb::Unary(unop, std::move(dst)));
    }

    void operator()(const ast::tacky::Binary& binary) {
        ast::asmb::Operand src1 = std::visit(*this, binary.mSrc1);
        ast::asmb::Operand src2 = std::visit(*this, binary.mSrc2);
        ast::asmb::Operand dst = std::visit(*this, binary.mDst);

        if (binary.mOp == ast::tacky::BinaryOperator::Divide) {
            mInstructions.emplace_back(ast::asmb::Mov(std::move(src1), ast::asmb::Reg(ast::asmb::RegisterName::AX)));
            mInstructions.emplace_back(ast::asmb::Cdq());
            mInstructions.emplace_back(ast::asmb::Idiv(std::move(src2)));
            mInstructions.emplace_back(ast::asmb::Mov(ast::asmb::Reg(ast::asmb::RegisterName::AX), std::move(dst)));
        }
        else if (binary.mOp == ast::tacky::BinaryOperator::Modulo) {
            mInstructions.emplace_back(ast::asmb::Mov(std::move(src1), ast::asmb::Reg(ast::asmb::RegisterName::AX)));
            mInstructions.emplace_back(ast::asmb::Cdq());
            mInstructions.emplace_back(ast::asmb::Idiv(std::move(src2)));
            mInstructions.emplace_back(ast::asmb::Mov(ast::asmb::Reg(ast::asmb::RegisterName::DX), std::move(dst)));
        }
        else {
            ast::asmb::BinaryOperator asmbOp = tacky_to_asmb_binop(binary.mOp);
            mInstructions.emplace_back(ast::asmb::Mov(std::move(src1), dst));
            mInstructions.emplace_back(ast::asmb::Binary(asmbOp, std::move(src2), std::move(dst)));
        }
    }

    // Function visitor
    ast::asmb::Function operator()(const ast::tacky::Function &func) {
        for (auto& instruction : func.mBody) {
            std::visit(*this, instruction);
        }
        return ast::asmb::Function(func.mIdentifier, std::move(this->mInstructions));
    }

    // Program visitor
    ast::asmb::Program operator()(const ast::tacky::Program &program) {
        return ast::asmb::Program((*this)(program.mFunction));
    }
};
    
}