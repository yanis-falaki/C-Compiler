#pragma once
#include <memory>
#include "ast/c_ast.hpp"
#include "lexer.hpp"

namespace compiler::parser {
    ast::c::Program parseProgram(lexer::LexList& lexList);
}