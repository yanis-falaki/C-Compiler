#pragma once

#include <string>
#include <vector>

namespace Compiler {

// ------------------------------> Type Enum <------------------------------

enum class LexType {
    Undefined,
    Identifier,
    Constant,
    Int,
    Void,
    Return,
    Open_Parenthesis,
    Close_Parenthesis,
    Open_Brace,
    Close_Brace,
    Semicolon
};

// ------------------------------> lexTypeToStr <------------------------------

inline constexpr std::string_view lexTypeToStr(LexType type) {
    switch (type) {
        case LexType::Undefined:          return "Undefined";
        case LexType::Identifier:         return "Identifier";
        case LexType::Constant:           return "Constant";
        case LexType::Int:                return "int";
        case LexType::Void:               return "void";
        case LexType::Return:             return "return";
        case LexType::Open_Parenthesis:   return "(";
        case LexType::Close_Parenthesis:  return ")";
        case LexType::Open_Brace:         return "{";
        case LexType::Close_Brace:        return "}";
        case LexType::Semicolon:          return ";";
        default:                          return "<invalid>";
    }
}

// ------------------------------> LexItem and KEYWORD_MAP <------------------------------

struct LexItem {
    LexType mLexType;
    std::string_view mSV;
};

class LexList {
private:
    std::vector<LexItem> mLexVector;
    size_t mCurrentIndex = 0;

public:

    size_t size() { return mLexVector.size(); }

    template<typename... Args>
    requires std::constructible_from<LexItem, Args...>
    void append(Args&&... args) {
        mLexVector.emplace_back(std::forward<Args>(args)...);
    }

    bool hasCurrent() const { return mCurrentIndex < mLexVector.size(); }

    /// @brief Get the current LexItem pointed to by the internal index
    const LexItem& current() const {
        if (!hasCurrent()) throw std::out_of_range("No more tokens");
        return mLexVector[mCurrentIndex];
    }

    /// @brief Get the current LexItem pointed to by the internal index and increment the internal index
    const LexItem& consume() {
        if (!hasCurrent()) throw std::out_of_range("No more tokens");
        ++mCurrentIndex;
        return mLexVector[mCurrentIndex-1];
    }

    /// @brief Reset the internal index to 0
    void resetIndex() {
        mCurrentIndex = 0;
    }

    /// @brief Increment the internal current index by 1.
    void advance() {
        ++mCurrentIndex;
    }
};

constexpr std::pair<std::string_view, LexType> KEYWORD_MAP[] = {
    std::make_pair("int", LexType::Int),
    std::make_pair("void", LexType::Void),
    std::make_pair("return", LexType::Return),
};

// ------------------------------> AST Definitions <------------------------------

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

// ------------------------------> Function Prototypes <------------------------------

LexList lexer(std::string_view preprocessed_input);
std::unique_ptr<Program> parseProgram(LexList& lexList);

}