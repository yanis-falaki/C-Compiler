#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ctre.hpp>
#include <stdexcept>
#include <format>
#include "compiler.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace Compiler {

// ------------------------------> Lexer Logic <------------------------------

// ------------------------------> checkForKeyword <------------------------------

static LexType checkForKeyword(std::string_view sv) {
    constexpr size_t len = std::size(KEYWORD_MAP);
    for (size_t i = 0; i < len; ++i) {
        auto [str, lexType] = KEYWORD_MAP[i];
        if (sv == str)
            return lexType;
    }
    // Fall back to identifier if not keyword
    return LexType::Identifier;
}

// ------------------------------> checkForType <------------------------------

static std::pair<LexType, std::string_view> checkForType(std::string_view sv) {
    static constexpr auto pattern = ctll::fixed_string{ R"((?<Identifier>[a-zA-Z_]\w*\b)|(?<Constant>[0-9]+\b))" };
    auto m = ctre::starts_with<pattern>(sv);
    if (m) {
        if (m.get<"Identifier">()) {
            return std::make_pair(LexType::Identifier, m);
        } else {
            return std::make_pair(LexType::Constant, m);
        }
    }
    else if(sv[0] == '(') return std::make_pair(LexType::Open_Parenthesis, "(");
    else if(sv[0] == ')') return std::make_pair(LexType::Close_Parenthesis, ")");
    else if(sv[0] == '{') return std::make_pair(LexType::Open_Brace, "{");
    else if(sv[0] == '}') return std::make_pair(LexType::Close_Brace, "}");
    else if(sv[0] == ';') return std::make_pair(LexType::Semicolon, ";");
    else return std::make_pair(LexType::Undefined, m);
}

// ------------------------------> lexer <------------------------------

/**
 * @brief Extracts a set of tokens from a string containing C source code.
 * 
 * @param preprocessed_input 
 */
LexList lexer(std::string_view preprocessed_input) {
    LexList lexList;

    // while not at end
    size_t pos = 0;
    size_t len = preprocessed_input.size();
    while (pos < len) {
        // remove leading whitespace
        if (std::isspace(preprocessed_input[pos])) {
            ++pos;
            continue;
        }

        // Check for lexType
        auto [lexType, token_sv] = checkForType(preprocessed_input.substr(pos, len-pos));

        // if no match is found, raise an error
        if (lexType == LexType::Undefined) {
            auto output_string = Utils::stringCenteredOnPos(preprocessed_input, pos, 30);
            throw std::runtime_error("Could not parse token at position " + std::to_string(pos)
                + "\nNearby text:\n" + output_string);
        }
        else if (lexType == LexType::Identifier) {
            LexType keyword = checkForKeyword(token_sv);
            lexList.append(keyword, token_sv);
        }
        else lexList.append(lexType, token_sv);

        pos += token_sv.size();
    }

    return lexList;
}

// ------------------------------> Language Construct Parsing <------------------------------

// ------------------------------> expect <------------------------------

const LexItem& expectAndAdvance(LexType expectedLexType, LexList& lexList) {
    const LexItem& actual = lexList.consume();
    if (actual.mLexType != expectedLexType) {
        auto errorString = std::format("Expected: {}, got {}", lexTypeToStr(expectedLexType), actual.mSV);
        throw std::runtime_error(errorString);
    }
    return actual;
}

const LexItem& expectNoAdvance(LexType expectedLexType, LexList& lexList) {
    const LexItem& actual = lexList.current();
    if (actual.mLexType != expectedLexType) {
        auto errorString = std::format("Expected: {}, got {}", lexTypeToStr(expectedLexType), actual.mSV);
        throw std::runtime_error(errorString);
    }
    return actual;
}

// ------------------------------> parseConstant <------------------------------

static std::unique_ptr<Constant> parseConstant(LexList& lexList) {
    const LexItem& lexConstant = expectAndAdvance(LexType::Constant, lexList);
    return std::make_unique<Constant>(std::stoi(std::string(lexConstant.mSV)));
}

// ------------------------------> parseExpression <------------------------------

static std::unique_ptr<Expression> parseExpession(LexList& lexList) {
    // Only works with constants for now
    const LexItem& lexExpression = lexList.current();
    if (lexExpression.mLexType == LexType::Constant)
        return parseConstant(lexList);

    std::string errorString = std::format("Expected an expression, instead got: {}", lexExpression.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseReturn <------------------------------

static std::unique_ptr<Return> parseReturn(LexList& lexList) {
    expectAndAdvance(LexType::Return, lexList);
    auto expression = parseExpession(lexList);
    expectAndAdvance(LexType::Semicolon, lexList);
    return std::make_unique<Return>(std::move(expression));
}

// ------------------------------> parseStatement <------------------------------

static std::unique_ptr<Statement> parseStatement(LexList& lexList) {
    // Parse statement
    const LexItem& lexStatement = lexList.current();
    if (lexStatement.mLexType == LexType::Return)
        return parseReturn(lexList);

    std::string errorString = std::format("Expected a statement, instead got: {}", lexStatement.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseFunction <------------------------------

static std::unique_ptr<Function> parseFunction(LexList& lexList) {
    // Check for keyword void
    expectAndAdvance(LexType::Int, lexList);

    // Check for identifier (should be main but we'll pay no attention for now)
    auto lexIdentifier =  expectAndAdvance(LexType::Identifier, lexList);

    // Check for parameter list
    expectAndAdvance(LexType::Open_Parenthesis, lexList);
    expectAndAdvance(LexType::Void, lexList);
    expectAndAdvance(LexType::Close_Parenthesis, lexList);

    // Opening brace
    expectAndAdvance(LexType::Open_Brace, lexList);

    // Statement
    auto statement = parseStatement(lexList);

    // Closing brace
    expectAndAdvance(LexType::Close_Brace, lexList);

    return std::make_unique<Function>(std::string(lexIdentifier.mSV), std::move(statement));
}

// ------------------------------> parseProgram <------------------------------

std::unique_ptr<Program> parseProgram(LexList& lexList) {
    // For now the program can only take a single function.
    auto function = parseFunction(lexList);

    if (lexList.hasCurrent())
        throw std::runtime_error("Program can only contain one top level function (for now)");

    return std::make_unique<Program>(std::move(function));
}

}
