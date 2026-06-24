#ifndef PARSER_H
#define PARSER_H

#include "ASTNode.h"
#include "Lexer.h"

#include <cstddef>
#include <string>
#include <vector>

class SyntaxError {
public:
    SyntaxError(std::string msg, int line, int column);
    const std::string& what() const;
    int getLine() const;
    int getColumn() const;

private:
    std::string message;
    int line;
    int column;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<ASTNode> analisar();

private:
    const std::vector<Token>& tokens;
    std::size_t pos;
    bool parsed;

    const Token& current() const;
    const Token& advance();
    bool check(const std::string& type) const;
    bool checkLexeme(const std::string& lexeme) const;
    bool isAtEnd() const;

    const Token& match(const std::string& expectedType, const std::string& label);
    const Token& matchLexeme(const std::string& expectedLexeme, const std::string& label);

    void erroSimboloInesperado(const std::string& esperado);
    void erroFimArquivo(const std::string& esperado);

    std::unique_ptr<ASTNode> programa();
    std::unique_ptr<ASTNode> bloco();
    std::unique_ptr<ASTNode> cmds();
    std::unique_ptr<ASTNode> cmd();
    std::unique_ptr<ASTNode> declaracao();
    std::unique_ptr<ASTNode> atribuicao();
    std::unique_ptr<ASTNode> condicional();
    std::unique_ptr<ASTNode> restoElse();
    std::unique_ptr<ASTNode> repeticaoFarmar();
    std::unique_ptr<ASTNode> parteRep();
    std::unique_ptr<ASTNode> atribRep();
    std::unique_ptr<ASTNode> repeticaoCombo();
    std::unique_ptr<ASTNode> selecaoSeletor();
    std::unique_ptr<ASTNode> quebraFraquezaCmd();
    std::unique_ptr<ASTNode> dropCmd();
    std::unique_ptr<ASTNode> printCmd();
    std::unique_ptr<ASTNode> listaParams();
    std::unique_ptr<ASTNode> coletarCmd();

    std::unique_ptr<ASTNode> expressao();
    std::unique_ptr<ASTNode> termoLogico();
    std::unique_ptr<ASTNode> restoExpr(std::unique_ptr<ASTNode> left);
    std::unique_ptr<ASTNode> restoTermo(std::unique_ptr<ASTNode> left);
    std::unique_ptr<ASTNode> exprAritmetica();
    std::unique_ptr<ASTNode> restoArit(std::unique_ptr<ASTNode> left);
    std::unique_ptr<ASTNode> termo();
    std::unique_ptr<ASTNode> restoTermoMul(std::unique_ptr<ASTNode> left);
    std::unique_ptr<ASTNode> fator();
};

#endif
