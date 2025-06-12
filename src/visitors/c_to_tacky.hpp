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

inline constexpr ast::tacky::UnaryOperator convert_unop(ast::c::UnaryOperator unop) {
    switch (unop) {
        case ast::c::UnaryOperator::Negate:          return ast::tacky::UnaryOperator::Negate;
        case ast::c::UnaryOperator::Complement:      return ast::tacky::UnaryOperator::Complement;
        default:
            throw std::runtime_error("convert_unop received an unknown ast::c::UnaryOperator");
            break;
    }
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
        auto tacky_op = convert_unop(unary.mOp);
        instructions.emplace_back(ast::tacky::Unary(tacky_op, src, dst));
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