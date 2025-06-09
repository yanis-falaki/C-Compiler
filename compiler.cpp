#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ctre.hpp>
#include <stdexcept>
#include "compiler.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace Compiler {

/**
 * @brief Extracts a set of tokens from a string containing C source code.
 * 
 * @param preprocessed_input 
 */
std::vector<LexItem> lexer(std::string_view preprocessed_input) {
    std::vector<LexItem> lexList;

    // while not at end
    size_t pos = 0;
    size_t len = preprocessed_input.size();
    while (pos < len) {
        // remove leading whitespace
        if (std::isspace(preprocessed_input[pos])) {
            ++pos;
            continue;
        }

        // Check for type
        auto [type, token_sv] = checkForType(preprocessed_input.substr(pos, len-pos));

        // if no match is found, raise an error
        if (type == Type::Undefined) {
            auto output_string = Utils::stringCenteredOnPos(preprocessed_input, pos, 30);
            throw std::runtime_error("Could not parse token at position " + std::to_string(pos)
                + "\nNearby text:\n" + output_string);
        }
        else if (type == Type::Identifier) {
            Type keyword = checkForKeyword(token_sv);
            lexList.emplace_back(keyword, token_sv);
        }
        else lexList.emplace_back(type, token_sv);

        pos += token_sv.size();
    }

    return lexList;
}


Type checkForKeyword(std::string_view sv) {
    constexpr size_t len = std::size(KEYWORD_MAP);
    for (size_t i = 0; i < len; ++i) {
        auto [str, type] = KEYWORD_MAP[i];
        if (sv == str)
            return type;
    }
    // Fall back to identifier if not keyword
    return Type::Identifier;
}

std::pair<Type, std::string_view> checkForType(std::string_view sv) {
    static constexpr auto pattern = ctll::fixed_string{ R"((?<Identifier>[a-zA-Z_]\w*\b)|(?<Constant>[0-9]+\b))" };
    auto m = ctre::starts_with<pattern>(sv);
    if (m) {
        if (m.get<"Identifier">()) {
            return std::make_pair(Type::Identifier, m);
        } else {
            return std::make_pair(Type::Constant, m);
        }
    }
    else if(sv[0] == '(') return std::make_pair(Type::Open_Parenthesis, "(");
    else if(sv[0] == ')') return std::make_pair(Type::Close_Parenthesis, ")");
    else if(sv[0] == '{') return std::make_pair(Type::Open_Brace, "{");
    else if(sv[0] == '}') return std::make_pair(Type::Close_Brace, "}");
    else if(sv[0] == ';') return std::make_pair(Type::Semicolon, ";");
    else return std::make_pair(Type::Undefined, m);
}

}
