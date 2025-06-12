#include "c_ast.hpp"
#include "asmb_ast.hpp"
#include <memory>

namespace compiler::codegen {

struct convertVisitor {
    // Expression visitors
    ast::asmb::Imm operator() (const ast::c::Constant& constant) const {
        return ast::asmb::Imm(constant.mValue);
    }

    // Statement visitors
    std::vector<ast::asmb::Instruction> operator() (const ast::c::Return& returnNode) const {
        std::vector<ast::asmb::Instruction> instructions(2);
        instructions[0] = ast::asmb::Mov(std::visit(*this, returnNode.mExpr), ast::asmb::Register("exp"));
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

}