#include "Lexer.h"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const std::string inputPath = argc > 1 ? argv[1] : "exemplos/exemplo.gs";

    try {
        const std::string source = readTextFile(inputPath);
        Lexer lexer(source);
        const LexicalResult result = lexer.analyze();

        std::cout << Lexer::formatResult(result);
        return result.errors.empty() ? 0 : 1;
    } catch (const std::exception& exception) {
        std::cerr << "Erro: " << exception.what() << '\n';
        return 2;
    }
}