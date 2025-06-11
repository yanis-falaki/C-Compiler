#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>

namespace compiler::ast::c {

// ------------------------------> Expressions <------------------------------

struct Expression {
    virtual int evaluate() const = 0;
    virtual ~Expression() = default;
    virtual void print(uint32_t depth=0) const = 0;
};

struct Constant : Expression {
    int mValue;
    Constant(int constant) : mValue(constant) {}
    int evaluate() const override { return mValue; }

    void print(uint32_t depth=0) const {
        auto indent = std::string(depth*2, ' ');
        std::cout << indent << "Constant: " << mValue << std::endl;
    }
};

// ------------------------------> Statements <------------------------------

struct Statement {
    virtual ~Statement() = default;
    virtual void print(uint32_t depth=0) const = 0;
};

struct Return : Statement {
    std::unique_ptr<Expression> mExpr;
    Return(std::unique_ptr<Expression> expr) : mExpr(std::move(expr)) {}

    void print(uint32_t depth=0) const {
        std::string indent = std::string(depth*2, ' ');
        std::cout << indent << "Return:" << std::endl;
        mExpr->print(depth + 1);
    }
};

struct If : Statement {
    std::unique_ptr<Expression> mCondition;
    std::unique_ptr<Statement> mThen;
    std::optional<std::unique_ptr<Statement>> mElse;

    If(std::unique_ptr<Expression> condition,
       std::unique_ptr<Statement> thenBranch,
       std::optional<std::unique_ptr<Statement>> elseBranch = std::nullopt)
        : mCondition(std::move(condition)),
          mThen(std::move(thenBranch)),
          mElse(std::move(elseBranch)) {}

    void print(uint32_t depth=0) const {
        std::string indent = std::string(depth*2, ' ');
        std::cout << indent << "If:" << std::endl;
        mCondition->print(depth+1);
        std::cout << indent << "Then:" << std::endl;
        mThen->print(depth+1);
        if (mElse.has_value()) {
            std::cout << indent << "Else:" << std::endl;
            mElse.value()->print(depth+1);
        }
    }
};

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    std::unique_ptr<Statement> mBody;

    Function(std::string identifier, std::unique_ptr<Statement> body)
        : mIdentifier(std::move(identifier)), mBody(std::move(body)) {}

    Function(std::unique_ptr<Statement> body)
        : mIdentifier(std::nullopt), mBody(std::move(body)) {}

    void print(uint32_t depth=0) const {
        std::string indent = std::string(depth*2, ' ');
        if (mIdentifier.has_value())
            std::cout << indent << "Function " << mIdentifier.value() << ":" << std::endl;
        else
            std::cout << indent << "Function " << ":" << std::endl;
        mBody->print(depth+1);
    }
};

// ------------------------------> Program Definition <------------------------------

struct Program {
    std::unique_ptr<Function> mFunction;

    Program(std::unique_ptr<Function> function)
        : mFunction(std::move(function)) {}
    
    void print(uint32_t depth=0) const {
        mFunction->print();
    }
};

}