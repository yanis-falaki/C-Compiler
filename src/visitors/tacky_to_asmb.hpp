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
        case ast::tacky::UnaryOperator::Logical_NOT:     throw std::invalid_argument("ast::tacky::Logical_NOT does not have an analog in ast::asmb");
    }
    throw std::runtime_error("tacky_to_asmb_unop received an unknown ast::tacky::UnaryOperator");
}

// ------------------------------> Map between TACKY binops and ASMB binops <------------------------------

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

// ------------------------------> Map between TACKY logical [bin/un]ops and ASMB condition codes <------------------------------

inline constexpr ast::asmb::ConditionCode tacky_binop_to_condition_code(ast::tacky::BinaryOperator binop) {
    switch (binop) {
        case ast::tacky::BinaryOperator::Is_Equal:          return ast::asmb::ConditionCode::E;
        case ast::tacky::BinaryOperator::Not_Equal:         return ast::asmb::ConditionCode::NE;
        case ast::tacky::BinaryOperator::Less_Than:         return ast::asmb::ConditionCode::L;  
        case ast::tacky::BinaryOperator::Greater_Than:      return ast::asmb::ConditionCode::G;
        case ast::tacky::BinaryOperator::Less_Or_Equal:     return ast::asmb::ConditionCode::LE;
        case ast::tacky::BinaryOperator::Greater_Or_Equal:  return ast::asmb::ConditionCode::GE;
    }
    throw std::invalid_argument("Invalid ast::tacky::BinaryOperator in tacky_binop_to_condition_code");
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

        // Logical NOT
        if (unary.mOp == ast::tacky::UnaryOperator::Logical_NOT) {
            mInstructions.emplace_back(ast::asmb::Cmp(ast::asmb::Imm(0), src));
            mInstructions.emplace_back(ast::asmb::Mov(ast::asmb::Imm(0), dst));
            mInstructions.emplace_back(ast::asmb::SetCC(ast::asmb::ConditionCode::E, dst));
            return;
        }

        // Standard unary op
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
        // Relational operators
        else if (ast::tacky::is_relational_binop(binary.mOp)) {
            ast::asmb::ConditionCode cc = tacky_binop_to_condition_code(binary.mOp);
            mInstructions.emplace_back(ast::asmb::Cmp(std::move(src2), std::move(src1)));
            mInstructions.emplace_back(ast::asmb::Mov(ast::asmb::Imm(0), dst));
            mInstructions.emplace_back(ast::asmb::SetCC(cc, std::move(dst)));
        }
        else {
            ast::asmb::BinaryOperator asmbOp = tacky_to_asmb_binop(binary.mOp);
            mInstructions.emplace_back(ast::asmb::Mov(std::move(src1), dst));
            mInstructions.emplace_back(ast::asmb::Binary(asmbOp, std::move(src2), std::move(dst)));
        }
    }

    void operator()(const ast::tacky::Copy& copy) {
        mInstructions.emplace_back(ast::asmb::Mov(
            std::visit(*this, copy.mSrc),
            std::visit(*this, copy.mDst)
        ));
    }

    void operator()(const ast::tacky::Jump& jump) {
        mInstructions.emplace_back(ast::asmb::Jmp(jump.mTarget));
    }

    void operator()(const ast::tacky::JumpIfZero& jmpIfZero) {
        mInstructions.emplace_back(ast::asmb::Cmp(ast::asmb::Imm(0), std::visit(*this, jmpIfZero.mCondition)));
        mInstructions.emplace_back(ast::asmb::JmpCC(ast::asmb::ConditionCode::E, jmpIfZero.mTarget));
    }

    void operator()(const ast::tacky::JumpIfNotZero& jmpIfNotZero) {
        mInstructions.emplace_back(ast::asmb::Cmp(ast::asmb::Imm(0), std::visit(*this, jmpIfNotZero.mCondition)));
        mInstructions.emplace_back(ast::asmb::JmpCC(ast::asmb::ConditionCode::NE, jmpIfNotZero.mTarget));
    }

    void operator()(const ast::tacky::JumpIfEqual& jmpIfEqual) {
        mInstructions.emplace_back(ast::asmb::Cmp(std::visit(*this, jmpIfEqual.mSrc1), std::visit(*this, jmpIfEqual.mSrc2)));
        mInstructions.emplace_back(ast::asmb::JmpCC(ast::asmb::ConditionCode::E, jmpIfEqual.mTarget));
    }

    void operator()(const ast::tacky::Label& label) {
        mInstructions.emplace_back(ast::asmb::Label(label.mIdentifier));
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