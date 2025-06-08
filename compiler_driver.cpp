#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <format>

namespace fs = std::filesystem;

fs::path preprocess_file(fs::path source_path, fs::path output_path, cxxopts::ParseResult args);
fs::path compile(fs::path source_path, fs::path output_path, cxxopts::ParseResult args);
void assemble(fs::path source_path, fs::path output_path, cxxopts::ParseResult args);

int main(int argc, char* argv[]) {
    cxxopts::Options options("Compiler Driver", "Driver for my C Compiler");
    options.add_options()
        ("s,source", "Source file", cxxopts::value<fs::path>())
        ("o,output", "Output File", cxxopts::value<fs::path>())
        ("P,no-linemarkers", "No linemarkers")
        ("E,preprocess", "Stop at preprocessing")
        ("S,assembly", "Stop at assembly generation")
        ("lex", "Stop at lexing")
        ("parse", "Stop at parsing")
        ("codegen", "Stop at assembly generation");

    options.parse_positional({"source"});

    auto args = options.parse(argc, argv);

    if (!args.count("source")) {
        std::cout << "Source file must be specified\n";
        return 1;
    }
    if (!args.count("output")) {
        std::cout << "Output file must be specified\n";
        return 1;  
    }

    fs::path source_path = args["source"].as<fs::path>();
    fs::path output_path = args["output"].as<fs::path>();


    auto preprocessed_path = preprocess_file(source_path, output_path, args);
    if (args.count("preprocess"))
        return 0;

    auto compiled_path = compile(preprocessed_path, output_path, args);
    fs::remove(preprocessed_path);
    if (compiled_path.empty() || args.count("assembly"))
        return 0;

    assemble(compiled_path, output_path, args);
    fs::remove(compiled_path);

    return 0;
}

fs::path preprocess_file(fs::path source_path, fs::path output_path, cxxopts::ParseResult args) {
    std::string dest_path = std::format("{}.i", output_path.string());

    std::string command;
    if (args.count("no-linemarkers")) {
        command = std::format("gcc -E -P {} -o {}", source_path.string(), dest_path);
    } else {
        command = std::format("gcc -E {} -o {}", source_path.string(), dest_path);
    }

    system(command.c_str());

    return fs::path(dest_path);
}

fs::path compile(fs::path source_path, fs::path output_path, cxxopts::ParseResult args) {
    if (args.count("parse") || args.count("lex") || args.count("lex"))
        return fs::path();

    std::string dest_path = std::format("{}.s", output_path.string());
    std::string command = std::format("gcc -S {} -o {}", source_path.string(), dest_path);
    system(command.c_str());
    return fs::path(dest_path);
}

void assemble(fs::path source_path, fs::path output_path, cxxopts::ParseResult args) {
    std::string command = std::format("gcc {} -o {}", source_path.string(), output_path.string());
    system(command.c_str());
}