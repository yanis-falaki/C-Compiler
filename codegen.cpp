#include "c_ast.hpp"
#include "asmb_ast.hpp"
#include <memory>

namespace compiler::codegen {

std::unique_ptr<ast::asmb::Program> lowerCToAsmb(std::unique_ptr<ast::c::Program> cProgramPtr) {
    auto& cFunction = cProgramPtr->mFunction;

    auto asmbProgramPtr = std::make_unique<ast::asmb::Program>();
    asmbProgramPtr->mFunction = std::make_unique<ast::asmb::Function>();
    auto& asmbFunction = asmbProgramPtr->mFunction;

    // Copy identifier
    asmbFunction->mIdentifier = cFunction->mIdentifier;

    // Create instructions
    asmbFunction->mInstructions = std::vector<std::unique_ptr<ast::asmb::Instruction>>();
}



}