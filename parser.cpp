#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ctre.hpp>
#include <stdexcept>
#include <format>
#include "lexer.hpp"
#include "c_ast.hpp"
#include "parser.hpp"
#include "utils.h"

namespace fs = std::filesystem;

namespace compiler::parser {

// ------------------------------> Language Construct Parsing <------------------------------

// ------------------------------> expect <------------------------------

const lexer::LexItem& expectAndAdvance(lexer::LexType expectedLexType, lexer::LexList& lexList) {
    const lexer::LexItem& actual = lexList.consume();
    if (actual.mLexType != expectedLexType) {
        auto errorString = std::format("Expected: {}, got {}", lexTypeToStr(expectedLexType), actual.mSV);
        throw std::runtime_error(errorString);
    }
    return actual;
}

const lexer::LexItem& expectNoAdvance(lexer::LexType expectedLexType, lexer::LexList& lexList) {
    const lexer::LexItem& actual = lexList.current();
    if (actual.mLexType != expectedLexType) {
        auto errorString = std::format("Expected: {}, got {}", lexTypeToStr(expectedLexType), actual.mSV);
        throw std::runtime_error(errorString);
    }
    return actual;
}

// ------------------------------> parseConstant <------------------------------

static std::unique_ptr<ast::c::Constant> parseConstant(lexer::LexList& lexList) {
    const lexer::LexItem& lexConstant = expectAndAdvance(lexer::LexType::Constant, lexList);
    return std::make_unique<ast::c::Constant>(std::stoi(std::string(lexConstant.mSV)));
}

// ------------------------------> parseExpression <------------------------------

static std::unique_ptr<ast::c::Expression> parseExpession(lexer::LexList& lexList) {
    // Only works with constants for now
    const lexer::LexItem& lexExpression = lexList.current();
    if (lexExpression.mLexType == lexer::LexType::Constant)
        return parseConstant(lexList);

    std::string errorString = std::format("Expected an expression, instead got: {}", lexExpression.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseReturn <------------------------------

static std::unique_ptr<ast::c::Return> parseReturn(lexer::LexList& lexList) {
    expectAndAdvance(lexer::LexType::Return, lexList);
    auto expression = parseExpession(lexList);
    expectAndAdvance(lexer::LexType::Semicolon, lexList);
    return std::make_unique<ast::c::Return>(std::move(expression));
}

// ------------------------------> parseStatement <------------------------------

static std::unique_ptr<ast::c::Statement> parseStatement(lexer::LexList& lexList) {
    // Parse statement
    const lexer::LexItem& lexStatement = lexList.current();
    if (lexStatement.mLexType == lexer::LexType::Return)
        return parseReturn(lexList);

    std::string errorString = std::format("Expected a statement, instead got: {}", lexStatement.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseFunction <------------------------------

static std::unique_ptr<ast::c::Function> parseFunction(lexer::LexList& lexList) {
    // Check for keyword void
    expectAndAdvance(lexer::LexType::Int, lexList);

    // Check for identifier (should be main but we'll pay no attention for now)
    auto lexIdentifier =  expectAndAdvance(lexer::LexType::Identifier, lexList);

    // Check for parameter list
    expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);
    expectAndAdvance(lexer::LexType::Void, lexList);
    expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);

    // Opening brace
    expectAndAdvance(lexer::LexType::Open_Brace, lexList);

    // Statement
    auto statement = parseStatement(lexList);

    // Closing brace
    expectAndAdvance(lexer::LexType::Close_Brace, lexList);

    return std::make_unique<ast::c::Function>(std::string(lexIdentifier.mSV), std::move(statement));
}

// ------------------------------> parseProgram <------------------------------

std::unique_ptr<ast::c::Program> parseProgram(lexer::LexList& lexList) {
    // For now the program can only take a single function.
    auto function = parseFunction(lexList);

    if (lexList.hasCurrent())
        throw std::runtime_error("Program can only contain one top level function (for now)");

    return std::make_unique<ast::c::Program>(std::move(function));
}

}
