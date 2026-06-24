# Analisador Sintatico GachaScript — Manual C++

Trabalho pratico de compiladores: analisador lexico e sintatico para a linguagem **GachaScript** (`.gs`), implementado manualmente em C++17, sem Flex, Lex, ANTLR ou `std::regex`.

## Como funciona

- **Analisador lexico**: reconhecimento de tokens caractere a caractere, por meio de automato implementado em codigo.
- **Analisador sintatico**: parser recursivo-descendente (top-down, LL(1)) que consome a lista de tokens e verifica a conformidade com a gramatica. Durante o parsing, uma arvore sintatica abstrata (AST) e construida e exibida.

---

## 1. Pre-requisitos

- CMake 3.15+
- Compilador C++17 (g++ via MinGW, ou MSVC via Visual Studio Build Tools)

```sh
cmake --version
g++ --version   # MinGW
```

---

## 2. Compilar e executar

### Compilar

```sh
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

### Linha de comando

```sh
.\build\analisador_sintatico_cli.exe exemplos\exemplo_valido.gs
```

A saida inclui:
- Lista de tokens (analise lexica)
- Resultado da analise sintatica (sucesso ou erro com linha/coluna)
- Arvore sintatica (AST) indentada

### Interface grafica (Windows)

```sh
.\build\analisador_gui.exe
```

| Botao | Acao |
|---|---|
| Abrir .gs | Carrega um arquivo fonte GachaScript |
| Compilar | Executa analise lexica + sintatica + exibe AST e erros |
| Carregar demo | Carrega o codigo de exemplo valido |
| Limpar | Limpa o editor e a saida |
| Salvar drop | Salva o resultado da analise em arquivo `.txt` |
| Sair | Fecha o programa |

---

## 3. Tokens reconhecidos (analisador lexico)

| Token | Lexema(s) |
|---|---|
| `Start` | `Start` |
| `Tipo_Var` | `Level`, `T4`, `T5`, `Meta`, `Lore`, `Assinatura` (com ou sem `~`) |
| `Cond_if` | `50/50` |
| `Cond_else` | `Garantido` |
| `Farmar` | `Farmar` |
| `Combo` | `Combo` |
| `Seletor` | `Seletor` |
| `Quebra_Fraqueza` | `Quebra_Fraqueza` |
| `Drop` | `Drop` |
| `Print` | `Anunciar` |
| `Coletar` | `Coletar` |
| `op_log` | `E`, `OU` |
| `op_rel` | `<`, `>`, `<=`, `>=`, `==`, `!=` |
| `op_mat` | `+`, `-`, `*`, `/` |
| `atrib` | `=` |
| `Num_int` | sequencia de digitos |
| `Identificador` | letra seguida de letras, digitos ou `_` |
| `texto` | cadeia entre aspas duplas |
| `abre_par` / `fecha_par` | `(` / `)` |
| `abre_chave` / `fecha_chave` | `{` / `}` |
| `ponto_virgula` | `;` |
| `virgula` | `,` |

### Comentarios

- `//` — comentario de linha
- `/* ... */` — comentario de bloco (bloco nao fechado gera erro)

### Erros lexicos detectados

- Simbolo nao pertencente a linguagem (ex.: `@`)
- Identificador mal formado (ex.: `j@`, `1a`)
- Tamanho do identificador excedido (max. 30 caracteres)
- Numero mal formado (ex.: `2.a3`)
- Tamanho excessivo do numero (max. 15 digitos)
- String com aspas duplas nao fechadas
- Char mal formado ou nao fechado
- Comentario de bloco nao fechado

---

## 4. Gramatica (regras sintaticas)

Gramatica LL(1) na notacao BNF utilizada pelo parser:

```
Programa        -> Start Bloco

Bloco           -> abre_chave Cmds fecha_chave

Cmds            -> Cmd Cmds | e

Cmd             -> Declaracao
                 | Atribuicao
                 | Condicional
                 | RepeticaoFarmar
                 | RepeticaoCombo
                 | SelecaoSeletor
                 | QuebraFraquezaCmd
                 | DropCmd
                 | PrintCmd
                 | ColetarCmd
                 | Bloco

Declaracao      -> Tipo_Var Identificador RestoDecl
RestoDecl       -> atrib Expressao ponto_virgula
                 | ponto_virgula

Atribuicao      -> Identificador atrib Expressao ponto_virgula

Condicional     -> Cond_if abre_par Expressao fecha_par Cmd RestoElse
RestoElse       -> Cond_else Cmd | e

RepeticaoFarmar -> Farmar abre_par ParteRep ponto_virgula Expressao
                     ponto_virgula AtribRep fecha_par Cmd

ParteRep        -> Tipo_Var Identificador atrib Expressao
                 | Identificador atrib Expressao
                 | e

AtribRep        -> Identificador atrib Expressao | e

RepeticaoCombo  -> Combo abre_par Expressao fecha_par Cmd

SelecaoSeletor  -> Seletor abre_par Expressao fecha_par Cmd

QuebraFraquezaCmd -> Quebra_Fraqueza ponto_virgula

DropCmd         -> Drop Expressao ponto_virgula
                 | Drop ponto_virgula

PrintCmd        -> Anunciar abre_par ListaParams fecha_par ponto_virgula
                 | Anunciar abre_par fecha_par ponto_virgula

ListaParams     -> Expressao RestoParams
RestoParams     -> virgula Expressao RestoParams | e

ColetarCmd      -> Coletar abre_par Identificador fecha_par ponto_virgula

Expressao       -> TermoLogico RestoExpr
RestoExpr       -> op_log TermoLogico RestoExpr | e

TermoLogico     -> ExprAritmetica RestoTermo
RestoTermo      -> op_rel ExprAritmetica | e

ExprAritmetica  -> Termo RestoArit
RestoArit       -> op_mat(+|-) Termo RestoArit | e

Termo           -> Fator RestoTermoMul
RestoTermoMul   -> op_mat(*|/) Fator RestoTermoMul | e

Fator           -> Num_int | texto | Identificador
                 | abre_par Expressao fecha_par
```

### Precedencia de operadores (da menor para maior)

| Nivel | Operadores |
|---|---|
| 1 (menor) | `E`, `OU` (logicos) |
| 2 | `<`, `>`, `<=`, `>=`, `==`, `!=` (relacionais) |
| 3 | `+`, `-` (aditivos) |
| 4 (maior) | `*`, `/` (multiplicativos) |

---

## 5. Erros sintaticos tratados

### 5.1 Parenteses/chaves abertos e nao fechados

O parser detecta quando falta `fecha_par` ou `fecha_chave` e reporta mensagens especificas:

- `"Fechamento de parenteses ')' esperado. Verifique se todos os parenteses estao fechados"`
- `"Fechamento de bloco '}' esperado. Verifique se todas as chaves foram fechadas corretamente"`

**Exemplo de codigo com erro:**

```c
Start {
    50/50 (x == 1 {   // falta ')' antes de '{'
        Drop;
    }
}
```

**Saida do erro:** `Linha 2, Coluna 19 - Fechamento de parenteses ')' na condicao do 50/50 esperado(a), encontrado abre_chave ("{")`

---

### 5.2 Sequencia de comandos fora de ordem

O parser identifica quando um comando nao pertence ao contexto gramatical.

- `"Comando inesperado: <tipo> (<lexema>). Verifique a sintaxe da instrucao nesta posicao"`

---

### 5.3 Comando ou expressao incompleta

| Cenario | Mensagem |
|---|---|
| Declaracao sem `;` | `"';' apos a declaracao. Declaracao incompleta"` |
| Atribuicao sem `;` | `"';' apos a atribuicao. Comando de atribuicao incompleto"` |
| Expressao vazia (`Level x = ;`) | `"Expressao esperada, encontrado ponto_virgula (';')"` |
| Programa truncado | `"<token> esperado(a), mas o programa terminou inesperadamente"` |

**Exemplo de codigo com erro:**

```c
Start {
    Level x = ;   // expressao vazia apos '='
}
```

**Saida do erro:** `Linha 2, Coluna 15 - Expressao esperada, encontrado ponto_virgula (";")`

---

### 5.4 Uso incorreto de operadores ou palavras-chave

| Cenario | Mensagem |
|---|---|
| `=` sem identificador antes | `"Identificador no lado esquerdo da atribuicao"` |
| `Farmar` sem `(` | `"Abertura de parenteses '(' apos 'Farmar'"` |
| `50/50` sem condicao | `"Abertura de parenteses '(' apos '50/50'"` |
| Palavra onde se espera expressao | `"Expressao esperada, encontrado <tipo>"` |

---

### 5.5 Token extra apos o fim do programa

Se houver tokens apos o fechamento do bloco principal:

- `"Token inesperado apos o fim do programa: <tipo> (<lexema>)"`

---

### 5.6 Erros lexicos (detectados antes da analise sintatica)

Se houver erros lexicos, a analise sintatica e abortada com a mensagem:

- `"Analise sintatica abortada: existem erros lexicos."`

---

## 6. Arvore sintatica (AST)

O parser constroi uma arvore sintatica abstrata durante a analise. Cada no contem:

- `type` — tipo do no (ex.: `Programa`, `Bloco`, `Declaracao`, `Expressao`, `Numero`, `Identificador`, etc.)
- `value` — valor associado (nome da variavel, valor do numero, operador)
- `line` / `column` — posicao no codigo-fonte
- `children` — lista de nos filhos

A arvore e exibida com indentacao hierarquica. Exemplo de saida:

```
Programa : Start
  Bloco
    Comandos
      Declaracao : Level
        Variavel : pity
          Tipo : Level
          Inicializacao : =
            Numero : 0
      Condicional : 50/50
        OpRelacional : ==
          Identificador : pity
          Numero : 89
        Bloco
          Comandos
            Drop : Drop
```

---

## 7. Implementacao do parser

- **Tipo**: parser recursivo-descendente (top-down)
- **Lookahead**: 1 token (LL(1))
- Cada regra da gramatica corresponde a um metodo da classe `Parser`
- A AST e construida incrementalmente durante o parsing
- Erros sao reportados com linha, coluna e mensagem descritiva em portugues
- O parser consome apenas os tokens validos produzidos pelo analisador lexico

---

## 8. Estrutura do projeto

```
AnalisadorLexicoCppManual/
├── CMakeLists.txt
├── README.md
├── documentacao.txt
├── exemplos/
│   ├── exemplo.gs              # codigo com erros lexicos propositais
│   ├── exemplo_valido.gs       # programa GachaScript valido
│   └── exemplo_invalido.gs     # codigo com erros sintaticos
└── src/
    ├── ASTNode.h               # estrutura da arvore sintatica (AST)
    ├── Lexer.h                 # Token, LexicalError, LexicalResult, classe Lexer
    ├── Lexer.cpp               # analisador lexico manual
    ├── Parser.h                # classe Parser e SyntaxError
    ├── Parser.cpp              # parser recursivo-descendente + AST
    ├── main_cli.cpp            # entrada via terminal
    └── gui_win32.cpp           # interface grafica Win32
```
