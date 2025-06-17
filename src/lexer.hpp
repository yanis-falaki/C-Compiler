#pragma once
#include <string>
#include <vector>
#include <memory>
#include <concepts>
#include <iostream>

namespace compiler::lexer {

// ------------------------------> Type Enum <------------------------------

enum class LexType {
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
    Semicolon,
    Plus,
    Asterisk,
    Forward_Slash,
    Percent,
    Left_Shift,
    Right_Shift,
    Bitwise_AND,
    Bitwise_OR,
    Bitwise_XOR,
    Logical_NOT,
    Logical_AND,
    Logical_OR,
    Equal_To,
    Not_Equal,
    Less_Than,
    Greater_Than,
    Less_Than_Or_Equal,
    Greater_Than_Or_Equal,
    Undefined, // Also serves as a count dummy enum to use for looping.
};

// ------------------------------> lex_type_to_str <------------------------------

inline constexpr std::string_view lex_type_to_str(LexType type) {
    switch (type) {
        case LexType::Undefined:                return "Undefined";
        case LexType::Identifier:               return "Identifier";
        case LexType::Constant:                 return "Constant";
        case LexType::Int:                      return "int";
        case LexType::Void:                     return "void";
        case LexType::Return:                   return "return";
        case LexType::BitwiseComplement:        return "~";
        case LexType::Negation:                 return "-";
        case LexType::Decrement:                return "--";
        case LexType::Open_Parenthesis:         return "(";
        case LexType::Close_Parenthesis:        return ")";
        case LexType::Open_Brace:               return "{";
        case LexType::Close_Brace:              return "}";
        case LexType::Semicolon:                return ";";
        case LexType::Plus:                     return "+";
        case LexType::Asterisk:                 return "*";
        case LexType::Forward_Slash:            return "/";
        case LexType::Percent:                  return "%";
        case LexType::Left_Shift:               return "<<";
        case LexType::Right_Shift:              return ">>";
        case LexType::Bitwise_AND:              return "&";
        case LexType::Bitwise_OR:               return "|";
        case LexType::Bitwise_XOR:              return "^";
        case LexType::Logical_NOT:              return "!";
        case LexType::Logical_AND:              return "&&";
        case LexType::Logical_OR:               return "||";
        case LexType::Equal_To:                 return "==";
        case LexType::Not_Equal:                return "!=";
        case LexType::Greater_Than:             return ">";
        case LexType::Less_Than:                return "<";
        case LexType::Greater_Than_Or_Equal:    return ">=";
        case LexType::Less_Than_Or_Equal:       return "<=";
    }
    throw std::runtime_error("lex_type_to_str received an unimplemented LexType");
}

// ------------------------------> Symbols to Check <------------------------------

// arraySize is length of enums - types we shouldn't check for.
constexpr size_t SYMBOL_MAPPING_SIZE = static_cast<int>(LexType::Undefined)-5;

constexpr std::array<std::pair<std::string_view, LexType>, SYMBOL_MAPPING_SIZE> generateSortedSymbolMapping() {
    std::array<std::pair<std::string_view, LexType>, SYMBOL_MAPPING_SIZE> sortedSymbols{};
    
    size_t arrayIndex = 0;  // Separate counter for array indexing
    for (int i = 0; i < static_cast<int>(LexType::Undefined); ++i) {
        LexType lexType = static_cast<LexType>(i);

        if (lexType == LexType::Identifier ||
            lexType == LexType::Constant ||
            lexType == LexType::Int ||
            lexType == LexType::Void ||
            lexType == LexType::Return) {
            continue;
        }

        sortedSymbols[arrayIndex] = std::pair(lex_type_to_str(lexType), lexType);
        arrayIndex++;  // Only increment when we actually add something
    }

    std::sort(sortedSymbols.begin(), sortedSymbols.end(),
              [](const auto& a, const auto& b) {
                return a.first.size() > b.first.size();
              });
    
    return sortedSymbols;
}

constexpr auto SORTED_SYMBOL_MAPPING = generateSortedSymbolMapping();

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
            os << i << ": " << lex_type_to_str(item.mLexType);
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