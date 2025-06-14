#pragma once
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>
#include <unordered_map>


namespace compiler::codegen {

// ------------------------------> Replace PseudoRegisters (1st Pass) <------------------------------

struct ReplacePseudoRegisters {

    std::unordered_map<std::string, uint32_t> mMap;
    int32_t mLastStackLocation = 0;
    
    // Operand visitors
    ast::asmb::Operand operator()(const ast::asmb::Imm& imm) const {
        return imm;
    }
    
    ast::asmb::Operand operator()(const ast::asmb::Reg& reg) const {
        return reg;
    }

    ast::asmb::Operand operator()(const ast::asmb::Pseudo& pseudo) {
        if (mMap.contains(pseudo.mName))
            return ast::asmb::Stack(mMap[pseudo.mName]);
        // else
        mLastStackLocation -= 4;
        mMap.emplace(pseudo.mName, mLastStackLocation);
        return ast::asmb::Stack(mLastStackLocation);
    }

    ast::asmb::Operand operator()(const ast::asmb::Stack& stack) const {
        return stack;
    }
    
    // Instruction visitors
    void operator()(const ast::asmb::Ret& ret) const {}
    
    void operator()(ast::asmb::Mov& mov) {
        mov.mSrc = std::visit(*this, mov.mSrc);
        mov.mDst = std::visit(*this, mov.mDst);
    }

    void operator()(ast::asmb::Unary& unary) {
        unary.mOperand = std::visit(*this, unary.mOperand);
    }

    void operator()(const ast::asmb::AllocateStack& allocateStack) const {
    }

    // Function visitor
    uint32_t operator()(ast::asmb::Function& func) {
        for (auto& instruction : func.mInstructions) {
            std::visit(*this, instruction);
        }
        return std::abs(mLastStackLocation);
    }

    // Program visitor
    uint32_t operator()(ast::asmb::Program& program) {
        (*this)(program.mFunction);
        return std::abs(mLastStackLocation);
    }
};

// ------------------------------> Fix up ASMB instructions (2nd Pass) <------------------------------

struct FixUpAsmbInstructions {
    std::vector<ast::asmb::Instruction>* mInstructions = nullptr;
    size_t mInstructionCounter = 0;
    uint32_t mStackSize;

    // Force visitor to have stackSize parameter.
    FixUpAsmbInstructions() = delete;
    FixUpAsmbInstructions(uint32_t stackSize) : mStackSize(stackSize) {}

    // Operand visitors
    void operator()(const ast::asmb::Imm& imm) const {
    }
    
    void operator()(const ast::asmb::Reg& reg) const {
    }

    void operator()(const ast::asmb::Pseudo& pseudo) const {
    }

    void operator()(const ast::asmb::Stack& stack) const {
    }
    
    // Instruction visitors
    void operator()(const ast::asmb::Ret& ret) const {}
    
    void operator()(ast::asmb::Mov& mov) {
        if (!std::holds_alternative<ast::asmb::Stack>(mov.mSrc) ||
            !std::holds_alternative<ast::asmb::Stack>(mov.mDst))
            return;
        // Otherwise this is a mem->mem mov operation which is not allowed.
        auto stackDst = mov.mDst;
        mov.mDst = ast::asmb::Reg(ast::asmb::RegisterName::R10);
        ast::asmb::Mov nextMovInstruction(mov.mDst, stackDst);
        mInstructions->emplace(mInstructions->begin() + mInstructionCounter + 1, std::move(nextMovInstruction));
    }

    void operator()(const ast::asmb::Unary& unary) const {
    }

    void operator()(const ast::asmb::AllocateStack& allocateStack) const {
    }

    // Function visitor
    void operator()(ast::asmb::Function& func) {
        // Set mInstructions reference
        mInstructions = &func.mInstructions;

        // Add AllocateStack instruction
        ast::asmb::AllocateStack allocateStackInstruction(mStackSize);
        mInstructions->emplace(mInstructions->begin(), allocateStackInstruction);

        // Fix any memory to memory mov instructions
        while(mInstructionCounter < mInstructions->size()) {
            std::visit(*this, (*mInstructions)[mInstructionCounter]);
            ++mInstructionCounter;
        }
    }

    // Program visitor
    void operator()(ast::asmb::Program& program) {
        (*this)(program.mFunction);
    }
};

// ------------------------------> Code Emission <------------------------------
// A lot of copying, but I'm okay with it for now as it makes it more readable.

struct EmitAsmbVisitor {

    // Operand visitors
    std::string operator() (const ast::asmb::Imm& imm) const {
        return std::format("${}", imm.mValue);
    }

    std::string operator() (const ast::asmb::Reg& reg) const {
        return std::string(ast::asmb::reg_name_to_operand(reg.mReg));
    }

    std::string operator() (const ast::asmb::Pseudo& pseudo) const {
        // should not have any pseudo registers.
        throw std::runtime_error("Pseudo operand in tree during EmitAsmbVisitor");
    }

    std::string operator() (const ast::asmb::Stack& stack) const {
        return std::format("{}(%rbp)", stack.mLocation);
    }

    // Instruction visitors
    std::string operator() (const ast::asmb::Mov& mov) const {
        return std::format("movl {}, {}", std::visit(*this, mov.mSrc), std::visit(*this, mov.mDst));
    }

    std::string operator() (const ast::asmb::Ret ret) const {
        return "movq %rbp, %rsp\n\tpopq %rbp\n\tret";
    }

    std::string operator() (const ast::asmb::Unary unary) const {
        return std::format("{} {}", ast::asmb::unary_op_to_instruction(unary.mOp), std::visit(*this, unary.mOperand));
    }

    std::string operator() (const ast::asmb::AllocateStack allocateStack) const {
        return std::format("subq ${}, %rsp", allocateStack.mValue);
    }

    // Function visitor
    std::string operator()(const ast::asmb::Function& function) {
        std::stringstream ss;
        ss << ".globl " << function.mIdentifier.value() << std::endl;
        ss << function.mIdentifier.value() << ":\n";
        ss << "\tpushq %rbp\n" << "\tmovq %rsp, %rbp\n";
        
        for (auto& instruction : function.mInstructions) {
            ss << "\t" << std::visit(*this, instruction) << "\n";
        }
    
        return ss.str();
    }

    // Program
    std::string operator()(const ast::asmb::Program& program) {
        return std::format("{}\n.section .note.GNU-stack,\"\",@progbits\n", (*this)(program.mFunction));
    }
};

}