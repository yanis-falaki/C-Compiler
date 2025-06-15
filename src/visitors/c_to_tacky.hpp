#pragma once
#include <memory>
#include <sstream>
#include <format>
#include <vector>
#include <stdexcept>
#include "../ast/ast_c.hpp"
#include "../ast/ast_tacky.hpp"

namespace compiler::codegen {

// ------------------------------> Helper function for temporary variables <------------------------------

inline std::string makeTemporary() {
    static uint32_t num;
    std::string returnString = std::format("tmp.{}", num);
    num += 1;
    return returnString;
}

// ------------------------------> Map between TACKY unops and C unops <------------------------------

inline constexpr ast::tacky::UnaryOperator c_to_tacky_unop(ast::c::UnaryOperator unop) {
    switch (unop) {
        case ast::c::UnaryOperator::Negate:          return ast::tacky::UnaryOperator::Negate;
        case ast::c::UnaryOperator::Complement:      return ast::tacky::UnaryOperator::Complement;
        default:
            throw std::runtime_error("c_to_tacky_unop received an unknown ast::c::UnaryOperator");
            break;
    }
}

// ------------------------------> Map between TACKY unops and C binops <------------------------------

inline constexpr ast::tacky::BinaryOperator c_to_tacky_binops(ast::c::BinaryOperator unop) {
    switch (unop) {
        case ast::c::BinaryOperator::Add:            return ast::tacky::BinaryOperator::Add;
        case ast::c::BinaryOperator::Subtract:       return ast::tacky::BinaryOperator::Subtract;
        case ast::c::BinaryOperator::Multiply:       return ast::tacky::BinaryOperator::Multiply;
        case ast::c::BinaryOperator::Divide:         return ast::tacky::BinaryOperator::Divide;
        case ast::c::BinaryOperator::Modulo:         return ast::tacky::BinaryOperator::Modulo;
    }
    throw std::runtime_error("c_to_tacky_binops received an unknown ast::c::UnaryOperator");
}

// ------------------------------> Conversion from C AST to TACKY AST <------------------------------

struct ConvertFromCToTacky {
    std::vector<ast::tacky::Instruction>& instructions;

    // Expression visitors
    ast::tacky::Val operator() (const ast::c::Constant& constant) {
        return ast::tacky::Constant(constant.mValue);
    }

    ast::tacky::Val operator() (const ast::c::Unary& unary) {
        ast::tacky::Val src = std::visit(*this, *unary.mExpr);
        std::string dstName = makeTemporary();
        ast::tacky::Var dst(dstName);
        auto tacky_op = c_to_tacky_unop(unary.mOp);
        instructions.emplace_back(ast::tacky::Unary(tacky_op, src, dst));
        return dst;
    }

    ast::tacky::Val operator() (const ast::c::Binary& binary) {
        ast::tacky::Val src1 = std::visit(*this, *binary.mLeft);
        ast::tacky::Val src2 = std::visit(*this, *binary.mRight);
        std::string dstName = makeTemporary();
        ast::tacky::Var dst(dstName);
        auto tacky_op = c_to_tacky_binops(binary.mOp);
        instructions.emplace_back(ast::tacky::Binary(tacky_op, src1, src2, dst));
        return dst;
    }

    // Statement visitors
    void operator() (const ast::c::Return& returnNode) {
        ast::tacky::Val src = std::visit(*this, returnNode.mExpr);
        instructions.emplace_back(ast::tacky::Return(src));
    }

    // Not yet implemented
    void operator() (const ast::c::If& ifNode) {
        return;
    }
};

inline ast::tacky::Function convertCFunctionToTacky(const ast::c::Function& functionNode) {
    std::vector<ast::tacky::Instruction> instructions;
    std::visit(ConvertFromCToTacky{instructions}, functionNode.mBody);

    if (functionNode.mIdentifier.has_value())
        return ast::tacky::Function(functionNode.mIdentifier.value(), std::move(instructions));
    else
        return ast::tacky::Function(makeTemporary(), std::move(instructions));
}

inline ast::tacky::Program convertCProgramToTacky(const ast::c::Program& program) {
    return ast::tacky::Program(convertCFunctionToTacky(program.mFunction));
}

}