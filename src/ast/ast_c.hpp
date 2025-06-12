#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>

namespace compiler::ast::c {

// ------------------------------> Unary Operator <------------------------------

enum class UnaryOperator {
    Complement,
    Negate
};



// ------------------------------> Expressions <------------------------------

struct Constant;
struct Unary;
using Expression = std::variant<Constant, Unary>;

struct Constant {
    int mValue;
    Constant(int constant) : mValue(constant) {}
};

struct Unary {
    UnaryOperator mOp;
    std::unique_ptr<Expression> mExpr;
    Unary(UnaryOperator op, std::unique_ptr<Expression> expr) : mOp(op), mExpr(std::move(expr)) {}
};

struct BitwiseComplement {
    std::unique_ptr<Expression> mExpr;
    BitwiseComplement(std::unique_ptr<Expression> expr): mExpr(std::move(expr)) {}
};

struct Negation {
    std::unique_ptr<Expression> mExpr;
    Negation(std::unique_ptr<Expression> expr): mExpr(std::move(expr)) {}
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