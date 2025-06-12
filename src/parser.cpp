#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ctre.hpp>
#include <stdexcept>
#include <format>
#include "lexer.hpp"
#include "ast/ast_c.hpp"
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

static ast::c::Constant parseConstant(lexer::LexList& lexList) {
    const lexer::LexItem& lexConstant = expectAndAdvance(lexer::LexType::Constant, lexList);
    return ast::c::Constant(std::stoi(std::string(lexConstant.mSV)));
}

// ------------------------------> parseUnaryOp <------------------------------

// forward declaration
static ast::c::Expression parseExpession(lexer::LexList& lexList);

static ast::c::UnaryOperator parseUnaryOp(lexer::LexList& lexList) {
    const lexer::LexItem& lexUnaryOp = lexList.current();
    lexList.advance();
    auto expression = std::make_unique<ast::c::Expression>(parseExpession(lexList));

    if (lexUnaryOp.mLexType == lexer::LexType::BitwiseComplement)
        return ast::c::BitwiseComplement(std::move(expression));
    else if (lexUnaryOp.mLexType == lexer::LexType::Negation)
        return ast::c::Negation(std::move(expression));
    
    std::string errorString = std::format("parseUnaryOp was called without an implementation for the UnaryOp");
    throw std::runtime_error(errorString);
}

// ------------------------------> parseExpression <------------------------------

static ast::c::Expression parseExpession(lexer::LexList& lexList) {
    // Only works with constants for now
    const lexer::LexItem& lexExpression = lexList.current();
    if (lexExpression.mLexType == lexer::LexType::Open_Parenthesis) {
        lexList.advance();
        auto expression = parseExpession(lexList);
        lexList.advance();
        return expression;
    }
    else if (lexExpression.mLexType == lexer::LexType::Constant)
        return parseConstant(lexList);
    else if (lexExpression.mLexType == lexer::LexType::BitwiseComplement ||
             lexExpression.mLexType == lexer::LexType::Negation ||
             lexExpression.mLexType == lexer::LexType::Decrement)
        return parseUnaryOp(lexList);


    std::string errorString = std::format("Expected an expression, instead got: {}", lexExpression.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseReturn <------------------------------

static ast::c::Return parseReturn(lexer::LexList& lexList) {
    expectAndAdvance(lexer::LexType::Return, lexList);
    auto returnObject = ast::c::Return(parseExpession(lexList));
    expectAndAdvance(lexer::LexType::Semicolon, lexList);
    return returnObject;
}

// ------------------------------> parseStatement <------------------------------

static ast::c::Statement parseStatement(lexer::LexList& lexList) {
    // Parse statement, only handles Return for now
    const lexer::LexItem& lexStatement = lexList.current();
    if (lexStatement.mLexType == lexer::LexType::Return)
        return parseReturn(lexList);

    std::string errorString = std::format("Expected a statement, instead got: {}", lexStatement.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseFunction <------------------------------

static ast::c::Function parseFunction(lexer::LexList& lexList) {
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

    // Create return function.
    auto returnedFunction = ast::c::Function(std::string(lexIdentifier.mSV), parseStatement(lexList));

    // Closing brace
    expectAndAdvance(lexer::LexType::Close_Brace, lexList);

    return returnedFunction;
}

// ------------------------------> parseProgram <------------------------------

ast::c::Program parseProgram(lexer::LexList& lexList) {
    // For now the program can only take a single function.
    auto returnedProgram = ast::c::Program(parseFunction(lexList));

    if (lexList.hasCurrent())
        throw std::runtime_error("Program can only contain one top level function (for now)");

    return returnedProgram;
}

}
