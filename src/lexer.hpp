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
    Increment,
    Int,
    Void,
    Return,
    If,
    Else,
    Go_To,
    Do,
    While,
    For,
    Break,
    Continue,
    Switch,
    Case,
    Default,
    Open_Parenthesis,
    Close_Parenthesis,
    Open_Brace,
    Close_Brace,
    Semicolon,
    Colon,
    Question_Mark,
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
    Is_Equal,
    Not_Equal,
    Less_Than,
    Greater_Than,
    Less_Or_Equal,
    Greater_Or_Equal,
    Assignment,
    Plus_Equal,
    Minus_Equal,
    Multiply_Equal,
    Divide_Equal,
    Modulo_Equal,
    AND_Equal,
    OR_Equal,
    XOR_Equal,
    Left_Shift_Equal,
    Right_Shift_Equal,
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
        case LexType::If:                       return "if";
        case LexType::Else:                     return "else";
        case LexType::Go_To:                    return "goto";
        case LexType::Do:                       return "do";
        case LexType::While:                    return "while";
        case LexType::For:                      return "for";
        case LexType::Break:                    return "break";
        case LexType::Continue:                 return "continue";
        case LexType::Switch:                   return "switch";
        case LexType::Case:                     return "case";
        case LexType::Default:                  return "default";
        case LexType::BitwiseComplement:        return "~";
        case LexType::Negation:                 return "-";
        case LexType::Decrement:                return "--";
        case LexType::Increment:                return "++";
        case LexType::Open_Parenthesis:         return "(";
        case LexType::Close_Parenthesis:        return ")";
        case LexType::Open_Brace:               return "{";
        case LexType::Close_Brace:              return "}";
        case LexType::Semicolon:                return ";";
        case LexType::Colon:                    return ":";
        case LexType::Question_Mark:            return "?";
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
        case LexType::Is_Equal:                 return "==";
        case LexType::Not_Equal:                return "!=";
        case LexType::Greater_Than:             return ">";
        case LexType::Less_Than:                return "<";
        case LexType::Greater_Or_Equal:         return ">=";
        case LexType::Less_Or_Equal:            return "<=";
        case LexType::Assignment:               return "=";
        case LexType::Plus_Equal:               return "+=";
        case LexType::Minus_Equal:              return "-=";
        case LexType::Multiply_Equal:           return "*=";
        case LexType::Divide_Equal:             return "/=";
        case LexType::Modulo_Equal:             return "%=";
        case LexType::AND_Equal:                return "&=";
        case LexType::OR_Equal:                 return "|=";
        case LexType::XOR_Equal:                return "^=";
        case LexType::Left_Shift_Equal:         return "<<=";
        case LexType::Right_Shift_Equal:        return ">>=";
    }
    throw std::runtime_error("lex_type_to_str received an unimplemented LexType");
}

// ------------------------------> KEYWORD_MAP <------------------------------

constexpr std::pair<std::string_view, LexType> KEYWORD_MAP[] = {
    std::make_pair("int", LexType::Int),
    std::make_pair("void", LexType::Void),
    std::make_pair("return", LexType::Return),
    std::make_pair("if", LexType::If),
    std::make_pair("else", LexType::Else),
    std::make_pair("goto", LexType::Go_To),
    std::make_pair("do", LexType::Do),
    std::make_pair("while", LexType::While),
    std::make_pair("for", LexType::For),
    std::make_pair("break", LexType::Break),
    std::make_pair("continue", LexType::Continue),
    std::make_pair("switch", LexType::Switch),
    std::make_pair("case", LexType::Case),
    std::make_pair("default", LexType::Default)
};

// ------------------------------> is_unary_op <------------------------------

inline bool is_lextype_unary_op(LexType type) {
    switch (type) {
        case LexType::BitwiseComplement:
        case LexType::Negation:
        case LexType::Logical_NOT:
            return true;
        default:
            return false;
    }
}

// ------------------------------> is_binary_op <------------------------------

// forward declaration
inline bool is_assignment(LexType type); 

inline bool is_lextype_binary_op(LexType type) {
    if (is_assignment(type)) return true;
    switch (type) {
        case LexType::Plus:
        case LexType::Negation:
        case LexType::Asterisk:
        case LexType::Forward_Slash:
        case LexType::Percent:
        case LexType::Left_Shift:
        case LexType::Right_Shift:
        case LexType::Bitwise_AND:
        case LexType::Bitwise_OR:
        case LexType::Bitwise_XOR:
        case LexType::Logical_AND:
        case LexType::Logical_OR:
        case LexType::Is_Equal:
        case LexType::Not_Equal:
        case LexType::Less_Than:
        case LexType::Greater_Than:
        case LexType::Less_Or_Equal:
        case LexType::Greater_Or_Equal:
        case LexType::Question_Mark:
            return true;
        default:
            return false;
    }
}

// ------------------------------> is_assignment <------------------------------

inline bool is_assignment(LexType type) {
    switch (type) {
        case LexType::Assignment:
        case LexType::Plus_Equal:
        case LexType::Minus_Equal:
        case LexType::Multiply_Equal:
        case LexType::Divide_Equal:
        case LexType::Modulo_Equal:
        case LexType::AND_Equal:
        case LexType::OR_Equal:
        case LexType::XOR_Equal:
        case LexType::Left_Shift_Equal:
        case LexType::Right_Shift_Equal:
            return true;
        default:
            return false;
    }
}

// ------------------------------> binary_op_precedence <------------------------------

inline constexpr uint32_t binary_op_precedence(LexType type) {
    switch (type) {
        case LexType::Asterisk:           return 50;
        case LexType::Forward_Slash:      return 50;
        case LexType::Percent:            return 50;
    
        case LexType::Plus:               return 45;
        case LexType::Negation:           return 45;
    
        case LexType::Left_Shift:         return 40;
        case LexType::Right_Shift:        return 40;
    
        case LexType::Less_Than:          return 35;
        case LexType::Greater_Than:       return 35;
        case LexType::Less_Or_Equal:      return 35;
        case LexType::Greater_Or_Equal:   return 35;
    
        case LexType::Is_Equal:           return 30;
        case LexType::Not_Equal:          return 30;
    
        case LexType::Bitwise_AND:        return 25;
        case LexType::Bitwise_XOR:        return 20;
        case LexType::Bitwise_OR:         return 15;
    
        case LexType::Logical_AND:        return 10;
        case LexType::Logical_OR:         return 5;

        case LexType::Question_Mark:      return 3;
        
        case LexType::Assignment:         return 1;
        case LexType::Plus_Equal:         return 1;
        case LexType::Minus_Equal:        return 1;
        case LexType::Multiply_Equal:     return 1;
        case LexType::Divide_Equal:       return 1;
        case LexType::Modulo_Equal:       return 1;
        case LexType::AND_Equal:          return 1;
        case LexType::OR_Equal:           return 1;
        case LexType::XOR_Equal:          return 1;
        case LexType::Left_Shift_Equal:   return 1;
        case LexType::Right_Shift_Equal:  return 1;
    }
    throw std::invalid_argument("Unhandled LexType in binary_op_precedence");
}

// ------------------------------> Symbols to Check <------------------------------

// arraySize is length of enums - types we shouldn't check for.
constexpr size_t SYMBOL_MAPPING_SIZE = static_cast<int>(LexType::Undefined)-16;

constexpr std::array<std::pair<std::string_view, LexType>, SYMBOL_MAPPING_SIZE> generateSortedSymbolMapping() {
    std::array<std::pair<std::string_view, LexType>, SYMBOL_MAPPING_SIZE> sortedSymbols{};
    
    size_t arrayIndex = 0;  // Separate counter for array indexing
    for (int i = 0; i < static_cast<int>(LexType::Undefined); ++i) {
        LexType lexType = static_cast<LexType>(i);

        if (lexType == LexType::Identifier ||
            lexType == LexType::Constant ||
            lexType == LexType::Int ||
            lexType == LexType::Void ||
            lexType == LexType::Return ||
            lexType == LexType::If ||
            lexType == LexType::Else ||
            lexType == LexType::Go_To ||
            lexType == LexType::Do ||
            lexType == LexType::While ||
            lexType == LexType::For ||
            lexType == LexType::Break ||
            lexType == LexType::Continue ||
            lexType == LexType::Switch ||
            lexType == LexType::Case ||
            lexType == LexType::Default) {
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

// ------------------------------> LexItem <------------------------------

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

    const LexItem& next() const {
        if (mCurrentIndex + 1 >= mLexVector.size())
            throw std::out_of_range("No next token");
        return mLexVector[mCurrentIndex+1];
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

// ------------------------------> Function Prototypes <------------------------------

LexList lexer(std::string_view preprocessed_input);

}