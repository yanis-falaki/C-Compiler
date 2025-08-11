#pragma once
#include "../../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>
#include <unordered_map>


namespace compiler::codegen {

using namespace ast;

// ------------------------------> Replace PseudoRegisters (1st Pass) <------------------------------

struct ReplacePseudoRegisters {

    std::unordered_map<std::string, uint32_t> mMap;
    int32_t mLastStackLocation = 0;
    
    // Operand visitors
    asmb::Operand operator()(const asmb::Imm& imm) const {
        return imm;
    }
    
    asmb::Operand operator()(const asmb::Reg& reg) const {
        return reg;
    }

    asmb::Operand operator()(const asmb::Pseudo& pseudo) {
        if (mMap.contains(pseudo.mName))
            return asmb::Stack(mMap[pseudo.mName]);
        // else
        mLastStackLocation -= 4;
        mMap.emplace(pseudo.mName, mLastStackLocation);
        return asmb::Stack(mLastStackLocation);
    }

    asmb::Operand operator()(const asmb::Stack& stack) const {
        return stack;
    }
    
    // Instruction visitors
    void operator()(asmb::Mov& mov) {
        mov.mSrc = std::visit(*this, mov.mSrc);
        mov.mDst = std::visit(*this, mov.mDst);
    }

    void operator()(asmb::Unary& unary) {
        unary.mOperand = std::visit(*this, unary.mOperand);
    }

    void operator()(asmb::Binary& binary) {
        binary.mOperand1 = std::visit(*this, binary.mOperand1);
        binary.mOperand2 = std::visit(*this, binary.mOperand2);
    }

    void operator()(asmb::Idiv& idiv) {
        idiv.mOperand = std::visit(*this, idiv.mOperand);
    }

    void operator()(const asmb::Cdq& cdq) const {}

    void operator()(const asmb::AllocateStack& allocateStack) const {
    }

    void operator()(const asmb::DeallocateStack& deallocateStack) const {}

    void operator()(asmb::Cmp& cmp) {
        cmp.mOperand1 = std::visit(*this, cmp.mOperand1);
        cmp.mOperand2 = std::visit(*this, cmp.mOperand2);
    }

    void operator()(const asmb::Jmp& jmp) const {}
    void operator()(const asmb::JmpCC& jmpCC) const {}

    void operator()(asmb::SetCC& setCC) {
        setCC.mDst = std::visit(*this, setCC.mDst);
    }

    void operator()(const asmb::Label& label) const {}

    void operator()(const asmb::Push& push) const {}

    void operator()(const asmb::Call& call) const {}

    void operator()(const asmb::Ret& ret) const {}

    // Function visitor
    void operator()(asmb::Function& func, SymbolInfo& symbolInfo) {
        mLastStackLocation = 0;
        mMap.clear();

        for (auto& instruction : func.mInstructions) {
            std::visit(*this, instruction);
        }
        symbolInfo.mStackSize = std::abs(mLastStackLocation);
    }

    // Program visitor
    void operator()(asmb::Program& program, SymbolMapType& symbolMap) {
        for (auto& function : program.mFunctions)
            (*this)(function, symbolMap.at(function.mIdentifier));
    }
};

// ------------------------------> Fix up ASMB instructions (2nd Pass) <------------------------------

struct FixUpAsmbInstructions {
    std::vector<asmb::Instruction>* mInstructions = nullptr;
    size_t mInstructionCounter = 0;

    // Operand visitors
    void operator()(const asmb::Imm& imm) const {}
    void operator()(const asmb::Reg& reg) const {}
    void operator()(const asmb::Pseudo& pseudo) const {}
    void operator()(const asmb::Stack& stack) const {}
    
    // Instruction visitors
    void operator()(const asmb::Ret& ret) const {}
    
    void operator()(asmb::Mov& mov) {
        if (!std::holds_alternative<asmb::Stack>(mov.mSrc) ||
            !std::holds_alternative<asmb::Stack>(mov.mDst))
            return;
        // Otherwise this is a mem->mem mov operation which is not allowed.
        auto stackDst = mov.mDst;
        mov.mDst = asmb::Reg(asmb::RegisterName::R10);
        asmb::Mov nextMovInstruction(mov.mDst, stackDst);
        mInstructions->emplace(mInstructions->begin() + mInstructionCounter + 1, std::move(nextMovInstruction));
    }

    void operator()(const asmb::Unary& unary) const {
    }

    void operator()(asmb::Binary& binary) {
        // Multiply operation can't have destination operand in memory
        if (binary.mOp == asmb::BinaryOperator::Multiply
            && std::holds_alternative<asmb::Stack>(binary.mOperand2)
        ) {
            auto stackDestination = binary.mOperand2;
            auto registerDst = asmb::Reg(asmb::RegisterName::R11);
            asmb::Mov mov1(stackDestination, registerDst);
            asmb::Mov mov2(registerDst, stackDestination);

            binary.mOperand2 = registerDst;
            mInstructions->emplace(mInstructions->begin() + mInstructionCounter + 1, std::move(mov2));
            mInstructions->emplace(mInstructions->begin() + mInstructionCounter, std::move(mov1));
        }

        // Shift operation needs second operand in CX register.
        else if ((binary.mOp == asmb::BinaryOperator::Left_Shift
                || binary.mOp == asmb::BinaryOperator::Right_Shift)
                && (!std::holds_alternative<asmb::Reg>(binary.mOperand1)
                || std::get<asmb::Reg>(binary.mOperand1).mReg != asmb::RegisterName::CX)
        ) {
            // Move count to CX register for calculation
            auto count = binary.mOperand1;
            auto registerDst = asmb::Reg(asmb::RegisterName::CX);
            asmb::Mov moveCount(count, registerDst);

            binary.mOperand1 = registerDst;
            mInstructions->emplace(mInstructions->begin() + mInstructionCounter, std::move(moveCount));
        }

        // Binary operation can't have both operands in memory.
        else if (std::holds_alternative<asmb::Stack>(binary.mOperand1)
            && std::holds_alternative<asmb::Stack>(binary.mOperand2)
        ) {
            // Move operand1 to r10 register
            auto registerDst = asmb::Reg(asmb::RegisterName::R10);
            asmb::Mov movInstruction(binary.mOperand1, registerDst);

            // Replace operand1 in binop as r10 register
            binary.mOperand1 = registerDst;

            // Insert move instruction
            mInstructions->emplace(mInstructions->begin() + mInstructionCounter, std::move(movInstruction));
        }
    }

    void operator()(asmb::Idiv& idiv) {
        if (!std::holds_alternative<asmb::Imm>(idiv.mOperand)) return;
        // Otherwise idiv is using an immediate value as operand which is not allowed.
        asmb::Mov preMovInstruction(idiv.mOperand, asmb::Reg(asmb::RegisterName::R10));
        idiv.mOperand = asmb::Reg(asmb::RegisterName::R10);
        mInstructions->emplace(mInstructions->begin() + mInstructionCounter, preMovInstruction);
    }

    void operator()(const asmb::Cdq& cdq) const {}

    void operator()(const asmb::AllocateStack& allocateStack) const {}

    void operator()(const asmb::DeallocateStack& deallocateStack) const {}

    void operator()(asmb::Cmp& cmp) const {

        // Compare operation can't have Operand2 be an immediate value (analaguous to dst in sub).
        if (std::holds_alternative<asmb::Imm>(cmp.mOperand2)) {
            asmb::Mov preMovInstruction(cmp.mOperand2, asmb::Reg(asmb::RegisterName::R10));
            cmp.mOperand2 = asmb::Reg(asmb::RegisterName::R10);
            mInstructions->emplace(mInstructions->begin() + mInstructionCounter, preMovInstruction);
        }

        // Compare operation can't have both operands in memory.
        // Second operand necessarily won't be in memory if last if statement was true as it was moved to R10 register.
        else if (std::holds_alternative<asmb::Stack>(cmp.mOperand1)
            && std::holds_alternative<asmb::Stack>(cmp.mOperand2)
        ) {
            // Move operand1 to r10 register
            auto registerDst = asmb::Reg(asmb::RegisterName::R10);
            asmb::Mov movInstruction(cmp.mOperand1, registerDst);

            // Replace operand1 in binop as r10 register
            cmp.mOperand1 = registerDst;

            // Insert move instruction
            mInstructions->emplace(mInstructions->begin() + mInstructionCounter, std::move(movInstruction));
        }
    }
    void operator()(const asmb::Jmp& jmp) const {}
    void operator()(const asmb::JmpCC& jmpCC) const {}
    void operator()(const asmb::SetCC& setCC) const {}
    void operator()(const asmb::Label& label) const {}
    void operator()(const asmb::Push& push) const {}
    void operator()(const asmb::Call& call) const {}

    // Function visitor
    void operator()(asmb::Function& func, uint32_t stackSize) {
        // Set mInstructions reference and reset instruction counter
        mInstructions = &func.mInstructions;
        mInstructionCounter = 0;

        // Add AllocateStack instruction rounded to nearest 16 for alignment
        stackSize = ((stackSize + 16 - 1) / 16) * 16;
        asmb::AllocateStack allocateStackInstruction(stackSize);
        mInstructions->emplace(mInstructions->begin(), allocateStackInstruction);

        // Fix any memory to memory mov instructions
        while(mInstructionCounter < mInstructions->size()) {
            std::visit(*this, (*mInstructions)[mInstructionCounter]);
            ++mInstructionCounter;
        }
    }

    // Program visitor
    void operator()(asmb::Program& program, SymbolMapType& symbolMap) {
        for (auto& function : program.mFunctions)
            (*this)(function, symbolMap.at(function.mIdentifier).mStackSize);
    }
};

// ------------------------------> Code Emission <------------------------------
// A lot of copying, but I'm okay with it for now as it makes it more readable.

struct EmitAsmbVisitor {

private:
    const SymbolMapType& mSymbolMap;

public:
    EmitAsmbVisitor(const SymbolMapType& symbolMap) : mSymbolMap(symbolMap) {}

    // Operand visitors
    std::string operator() (const asmb::Imm& imm) {
        return std::format("${}", imm.mValue);
    }

    std::string operator() (const asmb::Reg& reg, asmb::RegisterSize registerSize = asmb::RegisterSize::DWORD) {
        return std::string(asmb::reg_name_to_string(reg.mReg, registerSize));
    }

    std::string operator() (const asmb::Pseudo& pseudo) {
        // should not have any pseudo registers.
        throw std::runtime_error("Pseudo operand in tree during EmitAsmbVisitor");
    }

    std::string operator() (const asmb::Stack& stack) {
        return std::format("{}(%rbp)", stack.mLocation);
    }

    // Instruction visitors
    std::string operator() (const asmb::Mov& mov) {
        return std::format("movl {}, {}", std::visit(*this, mov.mSrc), std::visit(*this, mov.mDst));
    }

    std::string operator() (const asmb::Ret& ret) {
        return "movq %rbp, %rsp\n\tpopq %rbp\n\tret";
    }

    std::string operator() (const asmb::Unary& unary) {
        return std::format("{} {}", asmb::unary_op_to_instruction(unary.mOp), std::visit(*this, unary.mOperand));
    }

    std::string operator() (const asmb::Binary& binary) {
        return std::format("{} {}, {}",
                            asmb::binary_op_to_instruction(binary.mOp),
                            std::visit(*this, binary.mOperand1),
                            std::visit(*this, binary.mOperand2));
    }

    std::string operator() (const asmb::Idiv& idiv) {
        return std::format("idivl {}", std::visit(*this, idiv.mOperand));
    }

    std::string operator() (const asmb::Cdq& cdq) {
        return "Cdq";
    }

    std::string operator() (const asmb::AllocateStack& allocateStack) {
        return std::format("subq ${}, %rsp", allocateStack.mValue);
    }

    std::string operator()(const asmb::DeallocateStack& deallocateStack) {
       return std::format("addq ${}, %rsp", deallocateStack.mValue);
    }

    std::string operator()(const asmb::Cmp& cmp) {
        return std::format("cmpl {}, {}", std::visit(*this, cmp.mOperand1), std::visit(*this, cmp.mOperand2));
    }

    std::string operator()(const asmb::Jmp& jmp) {
        return std::format("jmp .L{}", jmp.mIdentifier);
    }

    std::string operator()(const asmb::JmpCC& jmpCC) {
        return std::format("j{} .L{}", asmb::condition_code_to_string(jmpCC.mCondCode), jmpCC.mIdentifier);
    }

    std::string operator()(const asmb::SetCC& setCC) {
        std::string_view dstString;
        // Can't use visitor on register as we need 1 byte name.
        if (std::holds_alternative<asmb::Reg>(setCC.mDst))
            dstString = asmb::reg_name_to_string(std::get<asmb::Reg>(setCC.mDst).mReg, 
                                                      asmb::RegisterSize::BYTE);
        else
            dstString = std::visit(*this, setCC.mDst);

        return std::format(
            "set{} {}",
            asmb::condition_code_to_string(setCC.mCondCode),
            dstString);
    }

    std::string operator()(const asmb::Label& label) {
        return std::format(".L{}:", label.mIdentifier);
    }

    std::string operator()(const asmb::Push& push) {
        std::string pushOperand;
        // If operand is a register it must use quad alias
        if (std::holds_alternative<asmb::Reg>(push.mOperand))
            pushOperand = (*this)(std::get<asmb::Reg>(push.mOperand), asmb::RegisterSize::QWORD);
        else
            pushOperand = std::visit(*this, push.mOperand);
        return "pushq " + pushOperand;
    }

    std::string operator()(const asmb::Call& call) {
        if (mSymbolMap.at(call.mFuncName).mDefined)
            return "call " + call.mFuncName;
        else
            return "call " + call.mFuncName + "@PLT";
    }

    // Function visitor
    void operator()(const asmb::Function& function, std::stringstream& ss) {   
        ss << ".globl " << function.mIdentifier << std::endl;
        ss << function.mIdentifier << ":\n";
        ss << "\tpushq %rbp\n" << "\tmovq %rsp, %rbp\n";
        
        for (auto& instruction : function.mInstructions) {
            ss << "\t" << std::visit(*this, instruction) << "\n";
        }
    }

    // Program
    std::string operator()(const asmb::Program& program) {
        std::stringstream ss;
        for (auto& function : program.mFunctions) {
            (*this)(function, ss);
            ss << "\n";
        }
        ss << ".section .note.GNU-stack,\"\",@progbits\n";
        return ss.str();
    }
};

}