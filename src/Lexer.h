#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

struct Token {
    std::string type;
    std::string lexeme;
    int line;
    int column;
};

struct LexicalError {
    std::string message;
    std::string lexeme;
    int line;
    int column;
};

struct LexicalResult {
    std::vector<Token> tokens;
    std::vector<LexicalError> errors;
};

class Lexer {
public:
    explicit Lexer(std::string sourceCode);

    LexicalResult analyze();

    static std::string formatResult(const LexicalResult& result);

private:
    static constexpr int MAX_IDENTIFIER_LENGTH = 30;
    static constexpr int MAX_NUMBER_DIGITS = 15;

    std::string source;
    std::size_t position;
    int line;
    int column;

    bool isAtEnd() const;
    char peek(std::size_t offset = 0) const;
    char advance();

    static bool isLetter(char value);
    static bool isDigit(char value);
    static bool isIdentifierStart(char value);
    static bool isIdentifierPart(char value);
    static bool isWhitespace(char value);
    static bool isDelimiter(char value);

    void skipIgnored(LexicalResult& result);
    void consumeComment(LexicalResult& result);
    void scanIdentifier(LexicalResult& result);
    void scanNumber(LexicalResult& result);
    void scanString(LexicalResult& result);
    void scanChar(LexicalResult& result);
    void scanSymbolOrOperator(LexicalResult& result);

    static std::string printable(const std::string& value);
};

std::string readTextFile(const std::string& path);
void writeTextFile(const std::string& path, const std::string& content);

#endif