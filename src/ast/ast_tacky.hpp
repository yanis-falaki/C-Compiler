#pragma once
#include <variant>
#include <cstdint>
#include <string>
#include <vector>

namespace compiler::ast::tacky {

// ------------------------------> Unary Operator <------------------------------

enum class UnaryOperator {
    Complement,
    Negate,
    Logical_NOT
};

constexpr std::string_view unary_op_to_string(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement:     return "Complement";
        case UnaryOperator::Negate:         return "Negate";
        case UnaryOperator::Logical_NOT:    return "Logical NOT";
    }
    throw std::invalid_argument("Unhandled UnaryOperator in unary_op_to_string");
}

// ------------------------------> Binary Operator <------------------------------

enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Left_Shift,
    Right_Shift,
    Bitwise_AND,
    Bitwise_OR,
    Bitwise_XOR,
    Is_Equal,
    Not_Equal,
    Less_Than,
    Greater_Than,
    Less_Or_Equal,
    Greater_Or_Equal
};

inline constexpr std::string_view binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:               return "Add";
        case BinaryOperator::Subtract:          return "Subtract";
        case BinaryOperator::Multiply:          return "Multiply";
        case BinaryOperator::Divide:            return "Divide";
        case BinaryOperator::Modulo:            return "Modulo";
        case BinaryOperator::Left_Shift:        return "Left Shift";
        case BinaryOperator::Right_Shift:       return "Right Shift";
        case BinaryOperator::Bitwise_AND:       return "Bitwise AND";
        case BinaryOperator::Bitwise_OR:        return "Bitwise OR";
        case BinaryOperator::Bitwise_XOR:       return "Bitwise XOR";
        case BinaryOperator::Is_Equal:          return "Is Equal";
        case BinaryOperator::Not_Equal:         return "Not Equal";
        case BinaryOperator::Less_Than:         return "Less Than";
        case BinaryOperator::Greater_Than:      return "Greater Than";
        case BinaryOperator::Less_Or_Equal:     return "Less or Equal";
        case BinaryOperator::Greater_Or_Equal:  return "Greater or Equal";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in ast::tacky::binary_op_to_string");
}

inline constexpr bool is_relational_binop(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Is_Equal:
        case BinaryOperator::Not_Equal:
        case BinaryOperator::Less_Than:
        case BinaryOperator::Less_Or_Equal:
        case BinaryOperator::Greater_Than:
        case BinaryOperator::Greater_Or_Equal:
            return true;
        default:
            return false;
    }
}

// ------------------------------> Val <------------------------------

struct Constant {
    uint32_t mValue;
    Constant(uint32_t value): mValue(value) {}
};

struct Var {
    std::string mIdentifier;
    Var(std::string identifier) : mIdentifier(std::move(identifier)) {}
};

using Val = std::variant<Constant, Var>;

// ------------------------------> Instruction <------------------------------

struct Return {
    Val mVal;
    Return(Val val) : mVal(std::move(val)) {}
};

struct Unary {
    UnaryOperator mOp;
    Val mSrc;
    Val mDst;
    Unary(UnaryOperator op, Val src, Val dst) : mOp(op), mSrc(std::move(src)), mDst(std::move(dst)) {}
};

struct Binary {
    BinaryOperator mOp;
    Val mSrc1;
    Val mSrc2;
    Val mDst;
    Binary(BinaryOperator op, Val src1, Val src2, Val dst)
        :   mOp(op),
            mSrc1(std::move(src1)),
            mSrc2(std::move(src2)),
            mDst(std::move(dst)) {}
};

struct Copy {
    Val mSrc;
    Val mDst;
    Copy(Val src, Val dst) : mSrc(std::move(src)), mDst(std::move(dst)) {}
};

struct Jump {
    std::string mTarget;
    Jump(std::string target) : mTarget(std::move(target)) {}
};

struct JumpIfZero {
    Val mCondition;
    std::string mTarget;
    JumpIfZero(Val condition, std::string target) : mCondition(std::move(condition)), mTarget(std::move(target)) {}
};

struct JumpIfNotZero {
    Val mCondition;
    std::string mTarget;
    JumpIfNotZero(Val condition, std::string target) : mCondition(std::move(condition)), mTarget(std::move(target)) {}
};

struct Label {
    std::string mIdentifier;
    Label(std::string identifier) : mIdentifier(std::move(identifier)) {}
};

using Instruction = std::variant<Return, Unary, Binary, Copy, Jump, JumpIfZero, JumpIfNotZero, Label>;

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::string mIdentifier;
    std::vector<Instruction> mBody;

    Function(std::string identifier, std::vector<Instruction> body) : 
        mIdentifier(std::move(identifier)),
        mBody(std::move(body)) {}
};

// ------------------------------> Program  <------------------------------

struct Program {
    Function mFunction;
    Program(Function function) : mFunction(std::move(function)) {}
};

}