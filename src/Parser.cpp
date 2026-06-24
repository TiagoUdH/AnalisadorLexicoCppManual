#include "Parser.h"

#include <sstream>
#include <stdexcept>
#include <utility>

// ============================ SyntaxError ============================

SyntaxError::SyntaxError(std::string msg, int line, int column)
    : message(std::move(msg)), line(line), column(column) {
}

const std::string& SyntaxError::what() const {
    return message;
}

int SyntaxError::getLine() const {
    return line;
}

int SyntaxError::getColumn() const {
    return column;
}

// ============================ Parser ============================

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), pos(0), parsed(false) {
}

const Token& Parser::current() const {
    return tokens[pos];
}

const Token& Parser::advance() {
    if (isAtEnd()) {
        static Token eofToken;
        eofToken.type = "EOF";
        eofToken.lexeme = "";
        eofToken.line = -1;
        eofToken.column = -1;
        return eofToken;
    }
    return tokens[pos++];
}

bool Parser::check(const std::string& type) const {
    if (isAtEnd()) return false;
    return current().type == type;
}

bool Parser::checkLexeme(const std::string& lexeme) const {
    if (isAtEnd()) return false;
    return current().lexeme == lexeme;
}

bool Parser::isAtEnd() const {
    return pos >= tokens.size();
}

void Parser::erroSimboloInesperado(const std::string& esperado) {
    if (isAtEnd()) {
        erroFimArquivo(esperado);
    }
    const Token& tok = current();
    std::ostringstream msg;
    msg << esperado << " esperado(a), encontrado " << tok.type
        << " (\"" << tok.lexeme << "\")";
    throw SyntaxError(msg.str(), tok.line, tok.column);
}

void Parser::erroFimArquivo(const std::string& esperado) {
    const Token& last = pos > 0 ? tokens[pos - 1] : Token{"", "", 0, 0};
    std::ostringstream msg;
    msg << esperado << " esperado(a), mas o programa terminou inesperadamente";
    throw SyntaxError(msg.str(), last.line, last.column);
}

const Token& Parser::match(const std::string& expectedType, const std::string& label) {
    if (isAtEnd()) {
        erroFimArquivo(label);
    }
    if (current().type != expectedType) {
        erroSimboloInesperado(label);
    }
    return advance();
}

const Token& Parser::matchLexeme(const std::string& expectedLexeme, const std::string& label) {
    if (isAtEnd()) {
        erroFimArquivo(label);
    }
    if (current().lexeme != expectedLexeme) {
        erroSimboloInesperado(label);
    }
    return advance();
}

// ============================ FORMAT AST ============================

std::string formatAST(const ASTNode& root, int indent) {
    std::ostringstream out;
    std::string pad(static_cast<std::size_t>(indent), ' ');
    out << pad << root.type;
    if (!root.value.empty()) {
        out << " : " << root.value;
    }
    out << "\n";
    for (const auto& child : root.children) {
        out << formatAST(*child, indent + 2);
    }
    return out.str();
}

// ============================ ENTRY POINT ============================

std::unique_ptr<ASTNode> Parser::analisar() {
    if (parsed) return nullptr;
    auto root = programa();
    if (!isAtEnd()) {
        const Token& tok = current();
        std::ostringstream msg;
        msg << "Token inesperado apos o fim do programa: " << tok.type
            << " (\"" << tok.lexeme << "\")";
        throw SyntaxError(msg.str(), tok.line, tok.column);
    }
    parsed = true;
    return root;
}

// ============================ GRAMMAR RULES ============================

std::unique_ptr<ASTNode> Parser::programa() {
    auto tok = match("Start", "Palavra reservada 'Start' (inicio do programa)");
    auto node = ASTNode::make("Programa", tok.lexeme, tok.line, tok.column);
    node->add(bloco());
    return node;
}

std::unique_ptr<ASTNode> Parser::bloco() {
    auto abre = match("abre_chave", "Abertura de bloco '{'");
    auto node = ASTNode::make("Bloco", "", abre.line, abre.column);
    node->add(cmds());
    match("fecha_chave", "Fechamento de bloco '}'. Verifique se todas as chaves foram fechadas corretamente");
    return node;
}

std::unique_ptr<ASTNode> Parser::cmds() {
    auto node = ASTNode::make("Comandos");
    while (!isAtEnd() && !check("fecha_chave")) {
        node->add(cmd());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::cmd() {
    if (isAtEnd()) {
        erroFimArquivo("Comando (declaracao, atribuicao, if, for, while, switch, etc.)");
    }

    const std::string& t = current().type;
    const std::string& l = current().lexeme;

    if (t == "Tipo_Var")       return declaracao();
    if (t == "Identificador")  return atribuicao();
    if (t == "Cond_if")        return condicional();
    if (t == "Farmar")         return repeticaoFarmar();
    if (t == "Combo")          return repeticaoCombo();
    if (t == "Seletor")        return selecaoSeletor();
    if (t == "Quebra_Fraqueza") return quebraFraquezaCmd();
    if (t == "Drop")           return dropCmd();
    if (t == "Print")          return printCmd();
    if (t == "Coletar")        return coletarCmd();
    if (t == "abre_chave")     return bloco();
    if (t == "ponto_virgula") {
        advance();
        return ASTNode::make("Vazio", ";");
    }

    std::ostringstream msg;
    msg << "Comando inesperado: " << t << " (\"" << l
        << "\"). Verifique a sintaxe da instrucao nesta posicao";
    throw SyntaxError(msg.str(), current().line, current().column);
}

std::unique_ptr<ASTNode> Parser::declaracao() {
    auto tipo = advance();
    auto node = ASTNode::make("Declaracao", tipo.lexeme, tipo.line, tipo.column);
    auto id = match("Identificador", "Nome da variavel apos o tipo");
    auto var = ASTNode::make("Variavel", id.lexeme, id.line, id.column);
    var->add(ASTNode::make("Tipo", tipo.lexeme, tipo.line, tipo.column));

    if (check("atrib")) {
        auto at = advance();
        auto exp = expressao();
        auto init = ASTNode::make("Inicializacao", at.lexeme, at.line, at.column);
        init->add(std::move(exp));
        var->add(std::move(init));
    }
    node->add(std::move(var));
    match("ponto_virgula", "';' apos a declaracao. Declaracao incompleta");
    return node;
}

std::unique_ptr<ASTNode> Parser::atribuicao() {
    auto id = match("Identificador", "Identificador no lado esquerdo da atribuicao");
    auto node = ASTNode::make("Atribuicao", id.lexeme, id.line, id.column);
    auto at = match("atrib", "Operador '=' de atribuicao. Use '=' para atribuir valor");
    node->add(expressao());
    match("ponto_virgula", "';' apos a atribuicao. Comando de atribuicao incompleto");
    return node;
}

std::unique_ptr<ASTNode> Parser::condicional() {
    auto tok = advance();
    auto node = ASTNode::make("Condicional", tok.lexeme, tok.line, tok.column);
    match("abre_par", "Abertura de parenteses '(' apos '50/50'");
    node->add(expressao());
    match("fecha_par", "Fechamento de parenteses ')' na condicao do 50/50");
    node->add(cmd());
    node->add(restoElse());
    return node;
}

std::unique_ptr<ASTNode> Parser::restoElse() {
    if (check("Cond_else")) {
        auto tok = advance();
        auto node = ASTNode::make("Senao", tok.lexeme, tok.line, tok.column);
        node->add(cmd());
        return node;
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::repeticaoFarmar() {
    auto tok = advance();
    auto node = ASTNode::make("LacoFarmar", tok.lexeme, tok.line, tok.column);
    match("abre_par", "Abertura de parenteses '(' apos 'Farmar'");
    auto init = parteRep();
    if (init) node->add(std::move(init));
    match("ponto_virgula", "';' apos a inicializacao do Farmar");
    node->add(expressao());
    match("ponto_virgula", "';' apos a condicao do Farmar");
    auto incr = atribRep();
    if (incr) node->add(std::move(incr));
    match("fecha_par", "Fechamento de parenteses ')' no cabecalho do Farmar");
    node->add(cmd());
    return node;
}

std::unique_ptr<ASTNode> Parser::parteRep() {
    if (check("Tipo_Var")) {
        auto tipo = advance();
        auto node = ASTNode::make("InicializacaoFor", tipo.lexeme, tipo.line, tipo.column);
        auto id = match("Identificador", "Identificador apos o tipo na inicializacao do Farmar");
        auto var = ASTNode::make("Variavel", id.lexeme, id.line, id.column);
        var->add(ASTNode::make("Tipo", tipo.lexeme, tipo.line, tipo.column));
        match("atrib", "'=' na inicializacao da variavel do Farmar");
        var->add(expressao());
        node->add(std::move(var));
        return node;
    }
    if (check("Identificador")) {
        auto id = advance();
        auto node = ASTNode::make("AtribuicaoFor", id.lexeme, id.line, id.column);
        match("atrib", "'=' na atribuicao inicial do Farmar");
        node->add(expressao());
        return node;
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::atribRep() {
    if (check("Identificador")) {
        auto id = advance();
        auto node = ASTNode::make("IncrementoFor", id.lexeme, id.line, id.column);
        match("atrib", "'=' no incremento do Farmar");
        node->add(expressao());
        return node;
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::repeticaoCombo() {
    auto tok = advance();
    auto node = ASTNode::make("LacoCombo", tok.lexeme, tok.line, tok.column);
    match("abre_par", "Abertura de parenteses '(' apos 'Combo'");
    node->add(expressao());
    match("fecha_par", "Fechamento de parenteses ')' na condicao do Combo");
    node->add(cmd());
    return node;
}

std::unique_ptr<ASTNode> Parser::selecaoSeletor() {
    auto tok = advance();
    auto node = ASTNode::make("SelecaoSeletor", tok.lexeme, tok.line, tok.column);
    match("abre_par", "Abertura de parenteses '(' apos 'Seletor'");
    node->add(expressao());
    match("fecha_par", "Fechamento de parenteses ')' na expressao do Seletor");
    node->add(cmd());
    return node;
}

std::unique_ptr<ASTNode> Parser::quebraFraquezaCmd() {
    auto tok = advance();
    auto node = ASTNode::make("QuebraFraqueza", tok.lexeme, tok.line, tok.column);
    match("ponto_virgula", "';' apos 'Quebra_Fraqueza'");
    return node;
}

std::unique_ptr<ASTNode> Parser::dropCmd() {
    auto tok = advance();
    auto node = ASTNode::make("Drop", tok.lexeme, tok.line, tok.column);
    if (!check("ponto_virgula")) {
        node->add(expressao());
    }
    match("ponto_virgula", "';' apos o comando Drop");
    return node;
}

std::unique_ptr<ASTNode> Parser::printCmd() {
    auto tok = advance();
    auto node = ASTNode::make("Print", tok.lexeme, tok.line, tok.column);
    match("abre_par", "Abertura de parenteses '(' apos 'Anunciar'");
    if (!check("fecha_par")) {
        node->add(listaParams());
    }
    match("fecha_par", "Fechamento de parenteses ')' nos parametros do Anunciar");
    match("ponto_virgula", "';' apos o comando Anunciar");
    return node;
}

std::unique_ptr<ASTNode> Parser::listaParams() {
    auto node = ASTNode::make("Parametros");
    node->add(expressao());
    while (check("virgula")) {
        advance();
        node->add(expressao());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::coletarCmd() {
    auto tok = advance();
    auto node = ASTNode::make("Coletar", tok.lexeme, tok.line, tok.column);
    match("abre_par", "Abertura de parenteses '(' apos 'Coletar'");
    auto id = match("Identificador", "Identificador da variavel para Coletar");
    node->add(ASTNode::make("Variavel", id.lexeme, id.line, id.column));
    match("fecha_par", "Fechamento de parenteses ')' no comando Coletar");
    match("ponto_virgula", "';' apos o comando Coletar");
    return node;
}

// ============================ EXPRESSIONS ============================

std::unique_ptr<ASTNode> Parser::expressao() {
    auto left = termoLogico();
    return restoExpr(std::move(left));
}

std::unique_ptr<ASTNode> Parser::restoExpr(std::unique_ptr<ASTNode> left) {
    if (check("op_log")) {
        auto op = advance();
        auto node = ASTNode::make("OpLogico", op.lexeme, op.line, op.column);
        node->add(std::move(left));
        auto right = termoLogico();
        node->add(std::move(right));
        return restoExpr(std::move(node));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::termoLogico() {
    auto left = exprAritmetica();
    return restoTermo(std::move(left));
}

std::unique_ptr<ASTNode> Parser::restoTermo(std::unique_ptr<ASTNode> left) {
    if (check("op_rel")) {
        auto op = advance();
        auto node = ASTNode::make("OpRelacional", op.lexeme, op.line, op.column);
        node->add(std::move(left));
        node->add(exprAritmetica());
        return node;
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::exprAritmetica() {
    auto left = termo();
    return restoArit(std::move(left));
}

std::unique_ptr<ASTNode> Parser::restoArit(std::unique_ptr<ASTNode> left) {
    while (check("op_mat") &&
           (current().lexeme == "+" || current().lexeme == "-")) {
        auto op = advance();
        auto node = ASTNode::make("OpAritmetico", op.lexeme, op.line, op.column);
        node->add(std::move(left));
        node->add(termo());
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::termo() {
    auto left = fator();
    return restoTermoMul(std::move(left));
}

std::unique_ptr<ASTNode> Parser::restoTermoMul(std::unique_ptr<ASTNode> left) {
    while (check("op_mat") &&
           (current().lexeme == "*" || current().lexeme == "/")) {
        auto op = advance();
        auto node = ASTNode::make("OpAritmetico", op.lexeme, op.line, op.column);
        node->add(std::move(left));
        node->add(fator());
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::fator() {
    if (check("Num_int")) {
        auto tok = advance();
        return ASTNode::make("Numero", tok.lexeme, tok.line, tok.column);
    }
    if (check("texto")) {
        auto tok = advance();
        return ASTNode::make("Texto", tok.lexeme, tok.line, tok.column);
    }
    if (check("Identificador")) {
        auto tok = advance();
        return ASTNode::make("Identificador", tok.lexeme, tok.line, tok.column);
    }
    if (check("abre_par")) {
        advance();
        auto node = ASTNode::make("Agrupamento");
        node->add(expressao());
        match("fecha_par", "Fechamento de parenteses ')' na expressao. Verifique se todos os parenteses estao fechados");
        return node;
    }
    if (isAtEnd()) {
        erroFimArquivo("Expressao (numero, texto, identificador ou '(')");
    }
    std::ostringstream msg;
    msg << "Expressao esperada, encontrado " << current().type
        << " (\"" << current().lexeme << "\")";
    throw SyntaxError(msg.str(), current().line, current().column);
}
