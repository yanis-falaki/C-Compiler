#pragma once
#include <string>
#include <vector>
#include <memory>
#include <concepts>
#include <iostream>

namespace compiler::lexer {

// ------------------------------> Type Enum <------------------------------

enum class LexType {
    Undefined,
    Identifier,
    Constant,
    BitwiseComplement,
    Negation,
    Decrement,
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
        case LexType::BitwiseComplement:  return "~";
        case LexType::Negation:           return "-";
        case LexType::Decrement:          return "--";
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

    /// @brief Print the contents of the list
    void print(std::ostream& os = std::cout) const {
        size_t i = 0;
        for (const auto& item : mLexVector) {
            os << i << ": " << lexTypeToStr(item.mLexType);
            if (item.mLexType == LexType::Identifier)
                std::cout << ": " << item.mSV << std::endl;
            else
                std::cout << std::endl;
            ++i;
        }
    }
};

constexpr std::pair<std::string_view, LexType> KEYWORD_MAP[] = {
    std::make_pair("int", LexType::Int),
    std::make_pair("void", LexType::Void),
    std::make_pair("return", LexType::Return),
};


// ------------------------------> Function Prototypes <------------------------------

LexList lexer(std::string_view preprocessed_input);

}