#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// Representa um token reconhecido pelo analisador lexico.
struct Token {
    std::string type;    // Tipo do token conforme a tabela da linguagem (ex.: "Tipo_Var", "op_rel")
    std::string lexeme;  // Sequencia de caracteres que formam o token no codigo-fonte
    int line;            // Linha em que o token comeca (base 1)
    int column;          // Coluna em que o token comeca (base 1)
};

// Representa um erro lexico encontrado durante a analise.
struct LexicalError {
    std::string message; // Descricao textual do erro detectado
    std::string lexeme;  // Trecho do codigo-fonte que causou o erro
    int line;            // Linha do erro (base 1)
    int column;          // Coluna do erro (base 1)
};

// Agrupa todos os tokens e erros produzidos em uma unica analise.
struct LexicalResult {
    std::vector<Token> tokens;        // Tokens validos reconhecidos
    std::vector<LexicalError> errors; // Erros lexicos detectados
};

// Analisador lexico manual para a linguagem GachaScript.
// Reconhece tokens percorrendo o codigo-fonte caractere a caractere,
// sem nenhuma biblioteca de regex ou gerador de scanner.
class Lexer {
public:
    // Constroi o analisador recebendo o codigo-fonte completo em memoria.
    explicit Lexer(std::string sourceCode);

    // Executa a analise lexica completa e retorna tokens e erros encontrados.
    LexicalResult analyze();

    // Formata tokens e erros em texto legivel para exibicao ou gravacao.
    static std::string formatResult(const LexicalResult& result);

private:
    // Comprimento maximo permitido para identificadores e nomes de variaveis.
    static constexpr int MAX_IDENTIFIER_LENGTH = 30;
    // Quantidade maxima de digitos em um literal inteiro.
    static constexpr int MAX_NUMBER_DIGITS = 15;

    std::string source;   // Codigo-fonte a ser analisado
    std::size_t position; // Posicao atual no codigo-fonte (indice em source)
    int line;             // Linha corrente (base 1)
    int column;           // Coluna corrente (base 1)

    bool isAtEnd() const;                      // true quando position >= source.size()
    char peek(std::size_t offset = 0) const;   // Le o caractere em position+offset sem consumir
    char advance();                            // Consome o caractere atual e atualiza linha/coluna

    static bool isLetter(char value);          // [A-Za-z]
    static bool isDigit(char value);           // [0-9]
    static bool isIdentifierStart(char value); // Primeiro caractere valido de um identificador (letra)
    static bool isIdentifierPart(char value);  // Caracteres validos no corpo (letra, digito, _)
    static bool isWhitespace(char value);      // Espacos, tabs, quebras de linha, etc.
    static bool isDelimiter(char value);       // Caracteres que encerram um token sem fazer parte dele

    // Avanca sobre espacos em branco e comentarios (// linha e /* bloco */).
    void skipIgnored(LexicalResult& result);
    // Consome comentario de bloco; registra erro se EOF antes do fechamento.
    void consumeComment(LexicalResult& result);
    // Reconhece identificadores, palavras reservadas e tipos de variavel (prefixo ~).
    void scanIdentifier(LexicalResult& result);
    // Reconhece literais inteiros; detecta numero mal formado e tamanho excedido.
    void scanNumber(LexicalResult& result);
    // Reconhece strings entre aspas duplas; registra erro se nao fechadas.
    void scanString(LexicalResult& result);
    // Trata aspas simples; char nao pertence a linguagem GachaScript (sempre gera erro).
    void scanChar(LexicalResult& result);
    // Reconhece operadores aritmeticos, relacionais, atribuicao e delimitadores.
    void scanSymbolOrOperator(LexicalResult& result);

    // Converte caracteres de controle em sequencias de escape legiveis (\n, \r, \t).
    static std::string printable(const std::string& value);
};

// Le o conteudo completo de um arquivo de texto em modo binario.
std::string readTextFile(const std::string& path);
// Grava conteudo em um arquivo de texto em modo binario.
void writeTextFile(const std::string& path, const std::string& content);

#endif