# Analisador Léxico Manual — GachaScript

Trabalho prático de compiladores: analisador léxico para a linguagem **GachaScript** (`.gs`), implementado manualmente em C++17, sem Flex, Lex, ANTLR ou `std::regex`.

## Como funciona

O reconhecimento de tokens é feito inteiramente no código, caractere a caractere, por meio de funções de estado dedicadas a cada categoria léxica. Não há gerador de scanner envolvido.

## Pré-requisitos

- CMake 3.15+
- Compilador C++17 (g++ via MinGW/MSYS2, ou MSVC via Visual Studio Build Tools)

Verifique com:

```sh
cmake --version
g++ --version   # MinGW/MSYS2
cl              # MSVC — execute no Developer Command Prompt
```

## Compilar

```sh
cmake -S . -B build
cmake --build build --config Release
```

## Executar

**Interface gráfica (Windows):**

```sh
.\build\Release\analisador_gui.exe
# ou, dependendo do gerador:
.\build\analisador_gui.exe
```

**Linha de comando:**

```sh
.\build\Release\analisador_cli.exe exemplos\exemplo.gs
# ou:
.\build\analisador_cli.exe exemplos\exemplo.gs
```

## Interface gráfica

| Botão | Ação |
|---|---|
| Abrir | Carrega um arquivo `.gs` |
| Analisar | Executa o analisador léxico |
| Exemplo | Carrega o código de demonstração |
| Limpar | Limpa o editor e a saída |
| Salvar saída | Salva tokens e erros em `.txt` |

## Tokens reconhecidos

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
| `Num_int` | sequência de dígitos |
| `Identificador` | letra seguida de letras, dígitos ou `_` |
| `texto` | cadeia entre aspas duplas |
| `abre_par` / `fecha_par` | `(` / `)` |
| `abre_chave` / `fecha_chave` | `{` / `}` |
| `ponto_virgula` | `;` |
| `virgula` | `,` |

## Comentários

- `//` — comentário de linha
- `/* ... */` — comentário de bloco (bloco não fechado gera erro)

> `{` e `}` são tokens de bloco na tabela da linguagem, por isso comentários **não** usam `{ }`.

## Erros léxicos detectados

- Símbolo não pertencente à linguagem (ex.: `@`)
- Identificador mal formado (ex.: `j@`, `1a`)
- Tamanho do identificador excedido (máx. 30 caracteres)
- Número mal formado (ex.: `2.a3`)
- Tamanho excessivo do número (máx. 15 dígitos)
- String com aspas duplas não fechadas
- Char mal formado ou não fechado
- Comentário de bloco não fechado

## Formato da saída

```
Linha: xx - Coluna xx - Token:<Tipo, Lexema>
```

**Exemplo:**

```
Linha: 1 - Coluna 1  - Token:<Start, Start>
Linha: 1 - Coluna 7  - Token:<abre_chave, {>
Linha: 2 - Coluna 5  - Token:<Tipo_Var, Level>
```

## Estrutura do projeto

```
AnalisadorLexicoCppManual/
├── CMakeLists.txt
├── exemplos/
│   └── exemplo.gs        # código de demonstração com erros propositais
└── src/
    ├── Lexer.h            # Token, LexicalError, LexicalResult, classe Lexer
    ├── Lexer.cpp          # implementação manual do analisador
    ├── main_cli.cpp       # entrada via terminal
    └── gui_win32.cpp      # interface gráfica Win32
```
