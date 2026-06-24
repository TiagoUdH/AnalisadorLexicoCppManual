#include "ASTNode.h"
#include "Lexer.h"
#include "Parser.h"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const std::string inputPath = argc > 1 ? argv[1] : "exemplos/exemplo_valido.gs";

    try {
        const std::string source = readTextFile(inputPath);
        Lexer lexer(source);
        const LexicalResult result = lexer.analyze();

        std::cout << "=== ANALISE LEXICA ===\n";
        std::cout << Lexer::formatResult(result);

        if (!result.errors.empty()) {
            std::cout << "Analise sintatica abortada: existem erros lexicos.\n";
            return 1;
        }

        std::cout << "=== ANALISE SINTATICA ===\n";
        Parser parser(result.tokens);
        auto ast = parser.analisar();

        std::cout << "Compilacao bem-sucedida! Nenhum erro sintatico encontrado.\n\n";

        std::cout << "=== ARVORE SINTATICA (AST) ===\n";
        std::cout << formatAST(*ast);

        return 0;
    } catch (const SyntaxError& error) {
        std::cerr << "\n=== ERRO SINTATICO ===\n";
        std::cerr << "Linha " << error.getLine()
                  << ", Coluna " << error.getColumn()
                  << " - " << error.what() << '\n';
        return 1;
    } catch (const std::exception& exception) {
        std::cerr << "Erro: " << exception.what() << '\n';
        return 2;
    }
}
