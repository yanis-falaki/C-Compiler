#pragma once
#include "../ast/ast_tacky.hpp"
#include "../ast/ast_asmb.hpp"
#include <memory>
#include <sstream>
#include <format>
#include <vector>
#include <unordered_map>
#include <array>


namespace compiler::codegen {

using namespace ast;

// ------------------------------> Map between TACKY unops and ASMB unops <------------------------------

inline constexpr asmb::UnaryOperator tacky_to_asmb_unop(tacky::UnaryOperator unop) {
    switch (unop) {
        case tacky::UnaryOperator::Negate:          return asmb::UnaryOperator::Negate;
        case tacky::UnaryOperator::Complement:      return asmb::UnaryOperator::Complement;
        case tacky::UnaryOperator::Logical_NOT:     throw std::invalid_argument("tacky::Logical_NOT does not have an analog in asmb");
    }
    throw std::runtime_error("tacky_to_asmb_unop received an unknown tacky::UnaryOperator");
}

// ------------------------------> Map between TACKY binops and ASMB binops <------------------------------

inline constexpr asmb::BinaryOperator tacky_to_asmb_binop(tacky::BinaryOperator unop) {
    switch (unop) {
        case tacky::BinaryOperator::Add:             return asmb::BinaryOperator::Add;
        case tacky::BinaryOperator::Subtract:        return asmb::BinaryOperator::Subtract;
        case tacky::BinaryOperator::Multiply:        return asmb::BinaryOperator::Multiply;
        case tacky::BinaryOperator::Left_Shift:      return asmb::BinaryOperator::Left_Shift;
        case tacky::BinaryOperator::Right_Shift:     return asmb::BinaryOperator::Right_Shift;
        case tacky::BinaryOperator::Bitwise_AND:     return asmb::BinaryOperator::Bitwise_AND;
        case tacky::BinaryOperator::Bitwise_OR:      return asmb::BinaryOperator::Bitwise_OR;
        case tacky::BinaryOperator::Bitwise_XOR:     return asmb::BinaryOperator::Bitwise_XOR;
    }
    throw std::runtime_error("tacky_to_asmb_binop received an unknown tacky::BinaryOperator");
}

// ------------------------------> Map between TACKY logical [bin/un]ops and ASMB condition codes <------------------------------

inline constexpr asmb::ConditionCode tacky_binop_to_condition_code(tacky::BinaryOperator binop) {
    switch (binop) {
        case tacky::BinaryOperator::Is_Equal:          return asmb::ConditionCode::E;
        case tacky::BinaryOperator::Not_Equal:         return asmb::ConditionCode::NE;
        case tacky::BinaryOperator::Less_Than:         return asmb::ConditionCode::L;  
        case tacky::BinaryOperator::Greater_Than:      return asmb::ConditionCode::G;
        case tacky::BinaryOperator::Less_Or_Equal:     return asmb::ConditionCode::LE;
        case tacky::BinaryOperator::Greater_Or_Equal:  return asmb::ConditionCode::GE;
    }
    throw std::invalid_argument("Invalid tacky::BinaryOperator in tacky_binop_to_condition_code");
}

// ------------------------------> TackyToAsmb (0th pass) <------------------------------

struct TackyToAsmb {

    std::vector<asmb::Instruction> mInstructions;
    
    // Val visitors
    asmb::Operand operator()(const tacky::Constant& constant) const {
        return asmb::Imm(constant.mValue);
    }

    asmb::Operand operator()(const tacky::Var& var) const {
        return asmb::Pseudo(var.mIdentifier);
    }

    // Instruction visitors
    void operator()(const tacky::Return& ret) {
        asmb::Operand src = std::visit(*this, ret.mVal);
        mInstructions.emplace_back(asmb::Mov(std::move(src), asmb::Reg(asmb::RegisterName::AX)));
        mInstructions.emplace_back(asmb::Ret());
    }

    void operator()(const tacky::Unary& unary) {
        asmb::Operand src = std::visit(*this, unary.mSrc);
        asmb::Operand dst = std::visit(*this, unary.mDst);

        // Logical NOT
        if (unary.mOp == tacky::UnaryOperator::Logical_NOT) {
            mInstructions.emplace_back(asmb::Cmp(asmb::Imm(0), src));
            mInstructions.emplace_back(asmb::Mov(asmb::Imm(0), dst));
            mInstructions.emplace_back(asmb::SetCC(asmb::ConditionCode::E, dst));
            return;
        }

        // Standard unary op
        asmb::UnaryOperator unop = tacky_to_asmb_unop(unary.mOp);
        mInstructions.emplace_back(asmb::Mov(std::move(src), dst));
        mInstructions.emplace_back(asmb::Unary(unop, std::move(dst)));
    }

    void operator()(const tacky::Binary& binary) {
        asmb::Operand src1 = std::visit(*this, binary.mSrc1);
        asmb::Operand src2 = std::visit(*this, binary.mSrc2);
        asmb::Operand dst = std::visit(*this, binary.mDst);

        if (binary.mOp == tacky::BinaryOperator::Divide) {
            mInstructions.emplace_back(asmb::Mov(std::move(src1), asmb::Reg(asmb::RegisterName::AX)));
            mInstructions.emplace_back(asmb::Cdq());
            mInstructions.emplace_back(asmb::Idiv(std::move(src2)));
            mInstructions.emplace_back(asmb::Mov(asmb::Reg(asmb::RegisterName::AX), std::move(dst)));
        }
        else if (binary.mOp == tacky::BinaryOperator::Modulo) {
            mInstructions.emplace_back(asmb::Mov(std::move(src1), asmb::Reg(asmb::RegisterName::AX)));
            mInstructions.emplace_back(asmb::Cdq());
            mInstructions.emplace_back(asmb::Idiv(std::move(src2)));
            mInstructions.emplace_back(asmb::Mov(asmb::Reg(asmb::RegisterName::DX), std::move(dst)));
        }
        // Relational operators
        else if (tacky::is_relational_binop(binary.mOp)) {
            asmb::ConditionCode cc = tacky_binop_to_condition_code(binary.mOp);
            mInstructions.emplace_back(asmb::Cmp(std::move(src2), std::move(src1)));
            mInstructions.emplace_back(asmb::Mov(asmb::Imm(0), dst));
            mInstructions.emplace_back(asmb::SetCC(cc, std::move(dst)));
        }
        else {
            asmb::BinaryOperator asmbOp = tacky_to_asmb_binop(binary.mOp);
            mInstructions.emplace_back(asmb::Mov(std::move(src1), dst));
            mInstructions.emplace_back(asmb::Binary(asmbOp, std::move(src2), std::move(dst)));
        }
    }

    void operator()(const tacky::Copy& copy) {
        mInstructions.emplace_back(asmb::Mov(
            std::visit(*this, copy.mSrc),
            std::visit(*this, copy.mDst)
        ));
    }

    void operator()(const tacky::Jump& jump) {
        mInstructions.emplace_back(asmb::Jmp(jump.mTarget));
    }

    void operator()(const tacky::JumpIfZero& jmpIfZero) {
        mInstructions.emplace_back(asmb::Cmp(asmb::Imm(0), std::visit(*this, jmpIfZero.mCondition)));
        mInstructions.emplace_back(asmb::JmpCC(asmb::ConditionCode::E, jmpIfZero.mTarget));
    }

    void operator()(const tacky::JumpIfNotZero& jmpIfNotZero) {
        mInstructions.emplace_back(asmb::Cmp(asmb::Imm(0), std::visit(*this, jmpIfNotZero.mCondition)));
        mInstructions.emplace_back(asmb::JmpCC(asmb::ConditionCode::NE, jmpIfNotZero.mTarget));
    }

    void operator()(const tacky::JumpIfEqual& jmpIfEqual) {
        mInstructions.emplace_back(asmb::Cmp(std::visit(*this, jmpIfEqual.mSrc1), std::visit(*this, jmpIfEqual.mSrc2)));
        mInstructions.emplace_back(asmb::JmpCC(asmb::ConditionCode::E, jmpIfEqual.mTarget));
    }

    void operator()(const tacky::Label& label) {
        mInstructions.emplace_back(asmb::Label(label.mIdentifier));
    }

    void operator()(const tacky::FuncCall& funcCall) {
        constexpr uint32_t maxArgs = asmb::ARG_REGISTERS.size();

        // Adjust stack aligment
        uint32_t numArgs = funcCall.mArgs.size();
        uint32_t registerArgs = numArgs > maxArgs ? maxArgs : numArgs;
        uint32_t stackArgs = numArgs - registerArgs;
        
        int stackPadding = 0;
        if (stackArgs % 2) // odd number of args on stack then add 8 padding for 16b alignment
            stackPadding = 8;

        if (stackPadding)
            mInstructions.emplace_back(asmb::AllocateStack(stackPadding));

        // Pass args in registers, regIndex = argIndex
        for (size_t regIndex = 0; regIndex < registerArgs; ++regIndex) {
            auto reg = asmb::ARG_REGISTERS[regIndex];
            auto assemblyArg = std::visit(*this, funcCall.mArgs[regIndex]);
            mInstructions.emplace_back(asmb::Mov(assemblyArg, reg));
        }

        // Pass args on stack, work backwards from the rightmost argument
        for (size_t argIndex = numArgs; argIndex-- > maxArgs;) {
            auto assemblyArg = std::visit(*this, funcCall.mArgs[argIndex]);
            if (std::holds_alternative<asmb::Reg>(assemblyArg) || std::holds_alternative<asmb::Imm>(assemblyArg))
                mInstructions.emplace_back(asmb::Push(assemblyArg));
            else {
                // Can't do memory to memory push
                mInstructions.emplace_back(asmb::Mov(assemblyArg, asmb::Reg(asmb::RegisterName::AX)));
                mInstructions.emplace_back(asmb::Push(asmb::Reg(asmb::RegisterName::AX)));
            }
        }

        // Emit call instruction
        mInstructions.emplace_back(asmb::Call(funcCall.mIdentifier));

        // Adjust stack pointer (each pushed arg takes 8 bytes)
        uint32_t bytesToRemove = (8 * stackArgs) + stackPadding;
        if (bytesToRemove > 0)
            mInstructions.emplace_back(asmb::DeallocateStack(bytesToRemove));

        // Retrive return value
        auto assemblyDst = std::visit(*this, funcCall.mDst);
        mInstructions.emplace_back(asmb::Mov(asmb::Reg(asmb::RegisterName::AX), assemblyDst));
    }

    // Function visitor
    asmb::Function operator()(const tacky::Function &func) {
        // functions are never nested so this is fine
        mInstructions.clear();

        // calculate amount of args in registers and stack
        constexpr uint32_t maxRegArgs = asmb::ARG_REGISTERS.size();
        uint32_t numParams = func.mParams.size();
        uint32_t registerArgs = numParams > maxRegArgs ? maxRegArgs : numParams;
        uint32_t stackArgs = numParams - registerArgs;

        // copy register args to stack
        for (size_t regIdx = 0; regIdx < registerArgs; ++regIdx)
            mInstructions.emplace_back(asmb::Mov(asmb::ARG_REGISTERS[regIdx], asmb::Pseudo(func.mParams[regIdx])));
        
        // copy remaining parameters from stack into current stack frame
        for (size_t stackArgIdx = 0; stackArgIdx < stackArgs; ++stackArgIdx)
            mInstructions.emplace_back(asmb::Mov(
                asmb::Stack(16 + stackArgIdx*8),
                asmb::Pseudo(func.mParams[maxRegArgs + stackArgIdx])
            ));
        
        // Write instructions for body
        for (auto& instruction : func.mBody) {
            std::visit(*this, instruction);
        }

        return asmb::Function(func.mIdentifier, std::move(mInstructions));
    }

    // Program visitor
    asmb::Program operator()(const tacky::Program &program) {
        std::vector<asmb::Function> functions;
        for (auto& function : program.mFunctions)
            functions.push_back((*this)(function));
        return asmb::Program(std::move(functions));
    }
};
    
}