#pragma once

#include <string>
#include <vector>

namespace Compiler {

enum class Type {
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

struct LexItem {
    Type mType;
    std::string_view mSV;
};

constexpr std::pair<std::string_view, Type> KEYWORD_MAP[] = {
    std::make_pair("int", Type::Int),
    std::make_pair("void", Type::Void),
    std::make_pair("return", Type::Return),
};

std::vector<LexItem> lexer(std::string_view preprocessed_input);
Type checkForKeyword(std::string_view sv);
std::pair<Type, std::string_view> checkForType(std::string_view sv);
}