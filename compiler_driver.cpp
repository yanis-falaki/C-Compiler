#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <format>
#include "utils.h"
#include "lexer.hpp"
#include "c_ast.hpp"
#include "parser.hpp"

namespace fs = std::filesystem;

fs::path preprocess_file(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args);
fs::path compile(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args);
void assemble(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args);

int main(int argc, char* argv[]) {
    cxxopts::Options options("Compiler Driver", "Driver for my C Compiler");
    options.add_options()
        ("s,source", "Source file", cxxopts::value<fs::path>())
        ("o,output", "Output File", cxxopts::value<fs::path>()->default_value("./a.out"))
        ("P,no-linemarkers", "No linemarkers")
        ("E,preprocess", "Stop at preprocessing")
        ("S,assembly", "Stop at assembly generation")
        ("lex", "Stop at lexing")
        ("parse", "Stop at parsing")
        ("codegen", "Stop at assembly generation");

    options.parse_positional({"source"});

    auto args = options.parse(argc, argv);

    if (!args.count("source")) {
        std::cerr << "Source file must be specified\n";
        return 1;
    }

    fs::path source_path = args["source"].as<fs::path>();
    fs::path output_path = args["output"].as<fs::path>();

    // Preprocessing Stage
    fs::path preprocessed_path;
    try {
        preprocessed_path = preprocess_file(source_path, output_path, args);
    } catch (const std::exception& e) {
        std::cerr << "Preprocessing failed: " << e.what() << std::endl;
        return 1;
    }

    if (args.count("preprocess"))
        return 0;

    // Compilation Stage
    fs::path compiled_path;
    try {
        compiled_path = compile(preprocessed_path, output_path, args);
    } catch(const std::exception& e) {
        std::cerr << "Compilation failed: " << e.what() << std::endl;
        fs::remove(preprocessed_path);
        return 1;
    }

    // Cleanup preprocessed file as it's no longer needed
    fs::remove(preprocessed_path);

    // Stop if -S or --assembly flag is set or incomplete compilation
    if (compiled_path.empty() || args.count("assembly"))
        return 0;

    // Assembling Stage
    try {
        assemble(compiled_path, output_path, args);
    } catch (const std::exception& e) {
        std::cerr << "Assembly failed: " << e.what() << std::endl;
        return 1;
    }

    // Cleanup assembly file as it's no longer needed
    fs::remove(compiled_path);

    return 0;
}


fs::path preprocess_file(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args) {
    std::string dest_path = std::format("{}.i", output_path.string());

    std::string command;
    if (args.count("no-linemarkers")) {
        command = std::format("gcc -E -P {} -o {}", source_path.string(), dest_path);
    } else {
        command = std::format("gcc -E -P {} -o {}", source_path.string(), dest_path);
    }

    if(system(command.c_str())) {
        throw std::runtime_error("Sys command error");
    }

    return fs::path(dest_path);
}


fs::path compile(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args) {
    if (args.count("codegen"))
        return fs::path();

    std::string sourceString = Utils::readFile(source_path);

    
    auto lexList = compiler::lexer::lexer(sourceString);
    if (args.count("lex")) return fs::path();

    auto program = compiler::parser::parseProgram(lexList);
    if (args.count("parse")) {
        compiler::ast::c::printAST(program);
        return fs::path();
    }

    std::string dest_path = std::format("{}.s", output_path.string());
    std::string command = std::format("gcc -S {} -o {}", source_path.string(), dest_path);
    if(system(command.c_str())) {
        throw std::runtime_error("Sys command error");
    }
    return fs::path(dest_path);
}


void assemble(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args) {
    std::string command = std::format("gcc {} -o {}", source_path.string(), output_path.string());
    if(system(command.c_str())) {
        throw std::runtime_error("Sys command error");
    }
}