#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include <cassert>

namespace compiler::ast::c {

// ------------------------------> Unary Operator <------------------------------

enum class UnaryOperator {
    Complement,
    Negate
};

inline constexpr std::string_view unary_op_to_string(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement: return "Complement";
        case UnaryOperator::Negate:     return "Negate";
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
    Bitwise_XOR
};

inline constexpr std::string_view binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:           return "Add";
        case BinaryOperator::Subtract:      return "Subtract";
        case BinaryOperator::Multiply:      return "Multiply";
        case BinaryOperator::Divide:        return "Divide";
        case BinaryOperator::Modulo:        return "Modulo";
        case BinaryOperator::Left_Shift:    return "Left Shift";
        case BinaryOperator::Right_Shift:   return "Right Shift";
        case BinaryOperator::Bitwise_AND:   return "Bitwise AND";
        case BinaryOperator::Bitwise_OR:    return "Bitwise OR";
        case BinaryOperator::Bitwise_XOR:   return "Bitwise XOR";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in binary_op_to_string");
}

inline constexpr uint32_t binary_op_precedence(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:           return 100;
        case BinaryOperator::Subtract:      return 100;
        case BinaryOperator::Multiply:      return 110;
        case BinaryOperator::Divide:        return 110;
        case BinaryOperator::Modulo:        return 110;
        case BinaryOperator::Left_Shift:    return 90;
        case BinaryOperator::Right_Shift:   return 90;
        case BinaryOperator::Bitwise_AND:   return 80;
        case BinaryOperator::Bitwise_OR:    return 60;
        case BinaryOperator::Bitwise_XOR:   return 70;
    }
    throw std::invalid_argument("Unhandled BinaryOperator in binary_op_precedence");
}

// ------------------------------> Expressions <------------------------------

struct Constant;
struct Unary;
struct Binary;
using Expression = std::variant<Constant, Unary, Binary>;

struct Constant {
    int mValue;
    Constant(int constant) : mValue(constant) {}
};

struct Unary {
    UnaryOperator mOp;
    std::unique_ptr<Expression> mExpr;
    Unary(UnaryOperator op, std::unique_ptr<Expression> expr) : mOp(op), mExpr(std::move(expr)) {}
};

struct Binary {
    BinaryOperator mOp;
    std::unique_ptr<Expression> mLeft;
    std::unique_ptr<Expression> mRight;
    Binary(BinaryOperator op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : mOp(op), mLeft(std::move(left)), mRight(std::move(right)) {}
};

// ------------------------------> Statements <------------------------------

// forward declarations
struct Return;
struct If;
using Statement = std::variant<Return, If>;

struct Return {
    Expression mExpr;
    Return(Expression expr) : mExpr(std::move(expr)) {}
};

struct If {
    Expression mCondition;
    std::unique_ptr<Statement> mThen;
    std::optional<std::unique_ptr<Statement>> mElse;

    If(Expression condition,
       std::unique_ptr<Statement> thenBranch,
       std::optional<std::unique_ptr<Statement>> elseBranch = std::nullopt)
        : mCondition(std::move(condition)),
          mThen(std::move(thenBranch)),
          mElse(std::move(elseBranch)) {}
};

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    Statement mBody;

    Function(std::optional<std::string> identifier, Statement body)
        : mIdentifier(identifier), mBody(std::move(body)) {}

    Function(Statement body)
        : mIdentifier(std::nullopt), mBody(std::move(body)) {}
};

// ------------------------------> Program Definition <------------------------------

struct Program {
    Function mFunction;

    Program(Function function)
        : mFunction(std::move(function)) {}
};

}