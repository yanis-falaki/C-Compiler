#pragma once
#include <memory>
#include "c_ast.hpp"
#include "lexer.hpp"

namespace compiler::parser {
    std::unique_ptr<ast::c::Program> parseProgram(lexer::LexList& lexList);
}