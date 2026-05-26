#include "Lexer.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

bool isWordCharacter(char value) {
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
           (value >= '0' && value <= '9') || value == '_';
}

std::string removeTypePrefix(const std::string& lexeme) {
    if (!lexeme.empty() && lexeme[0] == '~') {
        return lexeme.substr(1);
    }

    return lexeme;
}

bool isTypeKeyword(const std::string& value) {
    return value == "Level" || value == "T4" || value == "T5" ||
           value == "Meta" || value == "Lore" || value == "Assinatura";
}

std::string reservedTokenType(const std::string& lexeme) {
    const std::string value = removeTypePrefix(lexeme);

    if (isTypeKeyword(value)) {
        return "Tipo_Var";
    }
    if (lexeme != value) {
        return "";
    }
    if (value == "Start") {
        return "Start";
    }
    if (value == "Garantido") {
        return "Cond_else";
    }
    if (value == "Farmar") {
        return "Farmar";
    }
    if (value == "Combo") {
        return "Combo";
    }
    if (value == "Seletor") {
        return "Seletor";
    }
    if (value == "Quebra_Fraqueza") {
        return "Quebra_Fraqueza";
    }
    if (value == "Drop") {
        return "Drop";
    }
    if (value == "Anunciar") {
        return "Print";
    }
    if (value == "Coletar") {
        return "Coletar";
    }
    if (value == "E" || value == "OU") {
        return "op_log";
    }

    return "";
}

}  // namespace

Lexer::Lexer(std::string sourceCode)
    : source(std::move(sourceCode)), position(0), line(1), column(1) {
}

LexicalResult Lexer::analyze() {
    LexicalResult result;

    while (!isAtEnd()) {
        skipIgnored(result);

        if (isAtEnd()) {
            break;
        }

        const char current = peek();

        if (source.compare(position, 5, "50/50") == 0 && !isWordCharacter(peek(5))) {
            const int startLine = line;
            const int startColumn = column;
            std::string lexeme;
            for (int count = 0; count < 5; ++count) {
                lexeme += advance();
            }
            result.tokens.push_back({"Cond_if", lexeme, startLine, startColumn});
        } else if (current == '~' && isIdentifierStart(peek(1))) {
            scanIdentifier(result);
        } else if (isIdentifierStart(current)) {
            scanIdentifier(result);
        } else if (isDigit(current)) {
            scanNumber(result);
        } else if (current == '"') {
            scanString(result);
        } else if (current == '\'') {
            scanChar(result);
        } else {
            scanSymbolOrOperator(result);
        }
    }

    return result;
}

std::string Lexer::formatResult(const LexicalResult& result) {
    std::ostringstream output;

    output << "TOKENS\n";
    if (result.tokens.empty()) {
        output << "(nenhum token encontrado)\n";
    }

    for (const Token& token : result.tokens) {
        output << "Linha: " << token.line
               << " - Coluna " << token.column
               << " - Token:<" << token.type << ", " << printable(token.lexeme) << ">\n";
    }

    output << "\nERROS\n";
    if (result.errors.empty()) {
        output << "(nenhum erro lexico encontrado)\n";
    }

    for (const LexicalError& error : result.errors) {
        output << "Linha: " << error.line
               << " - Coluna " << error.column
               << " - Erro: " << error.message;

        if (!error.lexeme.empty()) {
            output << " (" << printable(error.lexeme) << ")";
        }

        output << '\n';
    }

    output << "\nResumo: " << result.tokens.size() << " token(s), "
           << result.errors.size() << " erro(s).\n";

    return output.str();
}

bool Lexer::isAtEnd() const {
    return position >= source.size();
}

char Lexer::peek(std::size_t offset) const {
    const std::size_t index = position + offset;
    if (index >= source.size()) {
        return '\0';
    }

    return source[index];
}

char Lexer::advance() {
    if (isAtEnd()) {
        return '\0';
    }

    const char current = source[position++];

    if (current == '\r') {
        if (!isAtEnd() && peek() == '\n') {
            position++;
        }
        line++;
        column = 1;
    } else if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }

    return current;
}

bool Lexer::isLetter(char value) {
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
}

bool Lexer::isDigit(char value) {
    return value >= '0' && value <= '9';
}

bool Lexer::isIdentifierStart(char value) {
    return isLetter(value);
}

bool Lexer::isIdentifierPart(char value) {
    return isLetter(value) || isDigit(value) || value == '_';
}

bool Lexer::isWhitespace(char value) {
    const unsigned char current = static_cast<unsigned char>(value);
    return value == ' ' || value == '\t' || value == '\n' || value == '\r' ||
           value == '\f' || value == '\v' || (current < 32 && value != '\0');
}

bool Lexer::isDelimiter(char value) {
    return value == '\0' || isWhitespace(value) || value == '{' || value == '}' ||
           value == '"' || value == '\'' || value == '+' || value == '-' ||
           value == '*' || value == '/' || value == '>' || value == '<' ||
           value == '=' || value == '!' || value == ';' || value == ',' ||
           value == '(' || value == ')' || value == '[' || value == ']';
}

void Lexer::skipIgnored(LexicalResult& result) {
    bool consumed;

    do {
        consumed = false;

        while (!isAtEnd() && isWhitespace(peek())) {
            advance();
            consumed = true;
        }

        if (!isAtEnd() && peek() == '/' && peek(1) == '/') {
            while (!isAtEnd() && peek() != '\n' && peek() != '\r') {
                advance();
            }
            consumed = true;
        } else if (!isAtEnd() && peek() == '/' && peek(1) == '*') {
            consumeComment(result);
            consumed = true;
        }
    } while (consumed && !isAtEnd());
}

void Lexer::consumeComment(LexicalResult& result) {
    const int startLine = line;
    const int startColumn = column;
    std::string lexeme;

    lexeme += advance();
    lexeme += advance();

    while (!isAtEnd()) {
        if (peek() == '*' && peek(1) == '/') {
            lexeme += advance();
            lexeme += advance();
            return;
        }

        lexeme += advance();
    }

    result.errors.push_back({
        "Fim de arquivo inesperado: comentario nao fechado",
        lexeme,
        startLine,
        startColumn
    });
}

void Lexer::scanIdentifier(LexicalResult& result) {
    const int startLine = line;
    const int startColumn = column;
    std::string lexeme;
    bool malformed = false;

    if (peek() == '~') {
        lexeme += advance();
    }

    if (!isIdentifierStart(peek())) {
        lexeme += advance();
        result.errors.push_back({
            "Identificador/variavel mal formado",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    while (!isAtEnd()) {
        const char current = peek();

        if (isIdentifierPart(current)) {
            lexeme += advance();
        } else if (isDelimiter(current)) {
            break;
        } else {
            malformed = true;
            lexeme += advance();

            while (!isAtEnd() && !isDelimiter(peek())) {
                lexeme += advance();
            }
            break;
        }
    }

    const std::string reservedType = reservedTokenType(lexeme);
    if (!reservedType.empty()) {
        result.tokens.push_back({reservedType, lexeme, startLine, startColumn});
        return;
    }

    if (malformed || (!lexeme.empty() && lexeme[0] == '~')) {
        result.errors.push_back({
            "Identificador/variavel mal formado",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    if (static_cast<int>(lexeme.size()) > MAX_IDENTIFIER_LENGTH) {
        result.errors.push_back({
            "Tamanho do identificador/variavel excedido",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    result.tokens.push_back({"Identificador", lexeme, startLine, startColumn});
}

void Lexer::scanNumber(LexicalResult& result) {
    const int startLine = line;
    const int startColumn = column;
    std::string lexeme;
    int digitCount = 0;

    while (!isAtEnd() && isDigit(peek())) {
        lexeme += advance();
        digitCount++;
    }

    if (!isAtEnd() && peek() == '.') {
        lexeme += advance();
        while (!isAtEnd() && !isDelimiter(peek())) {
            lexeme += advance();
        }

        result.errors.push_back({
            "Numero mal formado",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    if (!isAtEnd() && !isDelimiter(peek())) {
        while (!isAtEnd() && !isDelimiter(peek())) {
            lexeme += advance();
        }

        result.errors.push_back({
            "Identificador/variavel mal formado: nao pode iniciar com digito",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    if (digitCount > MAX_NUMBER_DIGITS) {
        result.errors.push_back({
            "Tamanho excessivo do numero",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    result.tokens.push_back({"Num_int", lexeme, startLine, startColumn});
}

void Lexer::scanString(LexicalResult& result) {
    const int startLine = line;
    const int startColumn = column;
    std::string lexeme;
    bool escaped = false;
    bool closed = false;

    lexeme += advance();

    while (!isAtEnd()) {
        const char current = peek();

        if (!escaped && (current == '\n' || current == '\r')) {
            break;
        }

        lexeme += advance();
        if (escaped) {
            escaped = false;
        } else if (current == '\\') {
            escaped = true;
        } else if (current == '"') {
            closed = true;
            break;
        }
    }

    if (!closed) {
        result.errors.push_back({
            "String mal formada: aspas duplas nao fechadas",
            lexeme,
            startLine,
            startColumn
        });
        return;
    }

    result.tokens.push_back({"texto", lexeme, startLine, startColumn});
}

void Lexer::scanChar(LexicalResult& result) {
    const int startLine = line;
    const int startColumn = column;
    std::string lexeme;
    bool escaped = false;
    bool closed = false;
    int logicalCharacters = 0;

    lexeme += advance();

    while (!isAtEnd()) {
        const char current = peek();

        if (!escaped && (current == '\n' || current == '\r')) {
            break;
        }

        lexeme += advance();
        if (escaped) {
            escaped = false;
            logicalCharacters++;
        } else if (current == '\\') {
            escaped = true;
        } else if (current == '\'') {
            closed = true;
            break;
        } else {
            logicalCharacters++;
        }
    }

    std::string message;
    if (!closed) {
        message = "Char mal formado: aspas simples nao fechadas";
    } else if (logicalCharacters != 1) {
        message = "Char mal formado: deve conter exatamente um caractere";
    } else {
        message = "Char nao pertence ao conjunto de simbolos terminais da linguagem";
    }

    result.errors.push_back({message, lexeme, startLine, startColumn});
}

void Lexer::scanSymbolOrOperator(LexicalResult& result) {
    const int startLine = line;
    const int startColumn = column;
    std::string lexeme;
    const char current = peek();

    switch (current) {
        case '+':
        case '-':
        case '*':
        case '/':
            lexeme += advance();
            result.tokens.push_back({"op_mat", lexeme, startLine, startColumn});
            break;
        case ';':
            lexeme += advance();
            result.tokens.push_back({"ponto_virgula", lexeme, startLine, startColumn});
            break;
        case ',':
            lexeme += advance();
            result.tokens.push_back({"virgula", lexeme, startLine, startColumn});
            break;
        case '(':
            lexeme += advance();
            result.tokens.push_back({"abre_par", lexeme, startLine, startColumn});
            break;
        case ')':
            lexeme += advance();
            result.tokens.push_back({"fecha_par", lexeme, startLine, startColumn});
            break;
        case '{':
            lexeme += advance();
            result.tokens.push_back({"abre_chave", lexeme, startLine, startColumn});
            break;
        case '}':
            lexeme += advance();
            result.tokens.push_back({"fecha_chave", lexeme, startLine, startColumn});
            break;
        case '>':
        case '<':
            lexeme += advance();
            if (!isAtEnd() && peek() == '=') {
                lexeme += advance();
            }
            result.tokens.push_back({"op_rel", lexeme, startLine, startColumn});
            break;
        case '=':
            lexeme += advance();
            if (!isAtEnd() && peek() == '=') {
                lexeme += advance();
                result.tokens.push_back({"op_rel", lexeme, startLine, startColumn});
            } else {
                result.tokens.push_back({"atrib", lexeme, startLine, startColumn});
            }
            break;
        case '!':
            lexeme += advance();
            if (!isAtEnd() && peek() == '=') {
                lexeme += advance();
                result.tokens.push_back({"op_rel", lexeme, startLine, startColumn});
            } else {
                result.errors.push_back({
                    "Simbolo nao pertencente ao conjunto de simbolos terminais da linguagem",
                    lexeme,
                    startLine,
                    startColumn
                });
            }
            break;
        default:
            lexeme += advance();
            result.errors.push_back({
                "Simbolo nao pertencente ao conjunto de simbolos terminais da linguagem",
                lexeme,
                startLine,
                startColumn
            });
            break;
    }
}

std::string Lexer::printable(const std::string& value) {
    std::ostringstream output;

    for (char current : value) {
        switch (current) {
            case '\n':
                output << "\\n";
                break;
            case '\r':
                output << "\\r";
                break;
            case '\t':
                output << "\\t";
                break;
            default:
                output << current;
                break;
        }

        if (output.tellp() > 80) {
            output << "...";
            break;
        }
    }

    return output.str();
}

std::string readTextFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void writeTextFile(const std::string& path, const std::string& content) {
    std::ofstream output(path, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Nao foi possivel gravar o arquivo: " + path);
    }

    output << content;
}