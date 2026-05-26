// Ponto de entrada do analisador lexico manual via linha de comando.
// Uso: analisador_cli [arquivo.gs]
// Se nenhum arquivo for informado, usa "exemplos/exemplo.gs" como padrao.
#include "Lexer.h"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Usa o primeiro argumento como caminho ou o arquivo-exemplo padrao
    const std::string inputPath = argc > 1 ? argv[1] : "exemplos/exemplo.gs";

    try {
        const std::string source = readTextFile(inputPath);
        Lexer lexer(source);
        const LexicalResult result = lexer.analyze();

        // Imprime tokens e erros na saida padrao
        std::cout << Lexer::formatResult(result);
        // Codigo de saida: 0 = sem erros lexicos, 1 = com erros lexicos, 2 = excecao
        return result.errors.empty() ? 0 : 1;
    } catch (const std::exception& exception) {
        std::cerr << "Erro: " << exception.what() << '\n';
        return 2;
    }
}