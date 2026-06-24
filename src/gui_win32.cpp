#include "ASTNode.h"
#include "Lexer.h"
#include "Parser.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <windows.h>
#include <commdlg.h>

namespace {

// ---------------------------------------------------------------------------
// Textos e identidade visual da janela
// ---------------------------------------------------------------------------
const char* APP_TITLE = "GachaScript Parser // Manual C++";
const char* APP_SUBTITLE = "Analisador lexico e sintatico: Start, Farmar, 50/50, Combo, Seletor e mais.";
const char* WINDOW_CLASS_NAME = "AnalisadorSintaticoCppWindow";
const char* WINDOW_TITLE = "GachaScript Parser - C++";
const char* ANALYZE_BUTTON_TEXT = "Compilar";

// ---------------------------------------------------------------------------
// Identificadores dos controles Win32
// ---------------------------------------------------------------------------
constexpr int ID_EDITOR   = 1001; // Caixa de texto do codigo-fonte (editavel)
constexpr int ID_OUTPUT   = 1002; // Caixa de texto da saida (somente leitura)
constexpr int ID_OPEN     = 1003; // Botao Abrir
constexpr int ID_ANALYZE  = 1004; // Botao Analisar
constexpr int ID_SAMPLE   = 1005; // Botao Exemplo
constexpr int ID_CLEAR    = 1006; // Botao Limpar
constexpr int ID_SAVE     = 1007; // Botao Salvar saida
constexpr int ID_TITLE    = 1008; // Titulo da tela
constexpr int ID_SUBTITLE = 1009; // Subtitulo da tela
constexpr int ID_EXIT     = 1010; // Botao Sair

// ---------------------------------------------------------------------------
// Paleta inspirada em uma interface de jogo/gacha: fundo escuro, dourado de
// drop raro, verde de sucesso e vermelho para erros lexicos.
// ---------------------------------------------------------------------------
const COLORREF THEME_BACKGROUND = RGB(13, 16, 30);
const COLORREF COLOR_PANEL = RGB(22, 27, 46);
const COLORREF COLOR_EDITOR = RGB(9, 12, 24);
const COLORREF COLOR_OUTPUT = RGB(14, 20, 33);
const COLORREF COLOR_STATUS = RGB(18, 24, 39);
const COLORREF COLOR_TEXT = RGB(232, 238, 247);
const COLORREF COLOR_MUTED = RGB(151, 164, 185);
const COLORREF COLOR_GOLD = RGB(255, 202, 92);
const COLORREF COLOR_GOLD_DARK = RGB(168, 116, 38);
const COLORREF COLOR_GREEN = RGB(78, 201, 115);
const COLORREF COLOR_RED = RGB(239, 96, 96);
const COLORREF COLOR_BUTTON = RGB(36, 45, 74);
const COLORREF COLOR_BUTTON_HOT = RGB(48, 61, 100);
const COLORREF COLOR_BUTTON_DOWN = RGB(30, 38, 63);

// ---------------------------------------------------------------------------
// Handles globais dos controles criados em createControls()
// ---------------------------------------------------------------------------
HWND gHeaderTitle = nullptr;    // Titulo visual da aplicacao
HWND gHeaderSubtitle = nullptr; // Subtitulo visual da aplicacao
HWND gEditor = nullptr;         // Area de edicao do codigo-fonte
HWND gOutput = nullptr;         // Area de saida dos tokens/erros
HWND gStatus = nullptr;         // Barra de status na parte inferior
HWND gEditorLabel = nullptr;    // Rotulo acima do editor
HWND gOutputLabel = nullptr;    // Rotulo acima da saida
HFONT gTitleFont = nullptr;     // Fonte do titulo
HFONT gUiFont = nullptr;        // Fonte proporcional para botoes e rotulos
HFONT gSmallFont = nullptr;     // Fonte do subtitulo e status
HFONT gMonoFont = nullptr;      // Fonte monoespaco para editor e saida
HBRUSH gBackgroundBrush = nullptr;
HBRUSH gPanelBrush = nullptr;
HBRUSH gEditorBrush = nullptr;
HBRUSH gOutputBrush = nullptr;
HBRUSH gStatusBrush = nullptr;

const char* SAMPLE_CODE =
    "Start {\r\n"
    "    Level pity = 0;\r\n"
    "    Level limite = 90;\r\n"
    "    Meta ganhou_50_50 = 0;\r\n"
    "\r\n"
    "    Farmar(pity = 0; pity < limite; pity = pity + 1) {\r\n"
    "        Anunciar(\"Realizando desejo no banner...\");\r\n"
    "\r\n"
    "        50/50 (pity == 89) {\r\n"
    "            Anunciar(\"Brilhou dourado!\");\r\n"
    "            Drop;\r\n"
    "        } Garantido {\r\n"
    "            Combo(pity < 5 OU pity != 0) {\r\n"
    "                Seletor(pity) {\r\n"
    "                    Anunciar(\"Sistema de pity ativo\");\r\n"
    "                    Quebra_Fraqueza;\r\n"
    "                }\r\n"
    "            }\r\n"
    "        }\r\n"
    "    }\r\n"
    "\r\n"
    "    Anunciar(\"Total de desejos realizados: \");\r\n"
    "    Anunciar(pity);\r\n"
    "}";

void deleteBrush(HBRUSH& brush) {
    if (brush != nullptr) {
        DeleteObject(brush);
        brush = nullptr;
    }
}

void deleteFont(HFONT& font) {
    if (font != nullptr) {
        DeleteObject(font);
        font = nullptr;
    }
}

// Cria recursos GDI usados no tema escuro.
void createThemeResources() {
    if (gBackgroundBrush == nullptr) {
        gBackgroundBrush = CreateSolidBrush(THEME_BACKGROUND);
        gPanelBrush = CreateSolidBrush(COLOR_PANEL);
        gEditorBrush = CreateSolidBrush(COLOR_EDITOR);
        gOutputBrush = CreateSolidBrush(COLOR_OUTPUT);
        gStatusBrush = CreateSolidBrush(COLOR_STATUS);
    }
}

// Libera fontes e pinceis GDI antes de encerrar a janela.
void destroyThemeResources() {
    deleteFont(gTitleFont);
    deleteFont(gUiFont);
    deleteFont(gSmallFont);
    deleteFont(gMonoFont);
    deleteBrush(gBackgroundBrush);
    deleteBrush(gPanelBrush);
    deleteBrush(gEditorBrush);
    deleteBrush(gOutputBrush);
    deleteBrush(gStatusBrush);
}

// Converte quebras de linha Unix (\n) para Windows (\r\n) antes de exibir no controle EDIT.
std::string toWindowsNewlines(const std::string& text) {
    std::string converted;
    converted.reserve(text.size() + 16);

    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == '\n' && (index == 0 || text[index - 1] != '\r')) {
            converted += '\r';
        }
        converted += text[index];
    }

    return converted;
}

// Le o texto atual de um controle EDIT e retorna como std::string.
std::string getControlText(HWND handle) {
    const int length = GetWindowTextLengthA(handle);
    std::string text(static_cast<std::size_t>(length) + 1, '\0');

    if (length > 0) {
        GetWindowTextA(handle, text.data(), length + 1);
    }

    text.resize(static_cast<std::size_t>(length));
    return text;
}

// Define o texto de um controle EDIT, convertendo quebras de linha.
void setControlText(HWND handle, const std::string& text) {
    const std::string converted = toWindowsNewlines(text);
    SetWindowTextA(handle, converted.c_str());
}

// Atualiza o texto exibido na barra de status.
void setStatus(const std::string& text) {
    if (gStatus != nullptr) {
        SetWindowTextA(gStatus, text.c_str());
    }
}

void showError(HWND owner, const std::string& message) {
    MessageBoxA(owner, message.c_str(), "GachaScript Parser", MB_ICONERROR | MB_OK);
}

void analyzeSource(HWND owner) {
    try {
        Lexer lexer(getControlText(gEditor));
        const LexicalResult result = lexer.analyze();

        std::ostringstream output;
        output << "=== ANALISE LEXICA ===\n";
        output << Lexer::formatResult(result);

        if (!result.errors.empty()) {
            output << "Analise sintatica abortada: existem erros lexicos.\n";
            setControlText(gOutput, output.str());
            setStatus("Compilacao abortada: " + std::to_string(result.errors.size()) + " erro(s) lexico(s).");
            return;
        }

        output << "\n=== ANALISE SINTATICA ===\n";

        try {
            Parser parser(result.tokens);
            auto ast = parser.analisar();
            output << "Compilacao bem-sucedida! Nenhum erro sintatico encontrado.\n";
            output << "\n=== ARVORE SINTATICA (AST) ===\n";
            output << formatAST(*ast);
            setStatus("Compilacao bem-sucedida! " + std::to_string(result.tokens.size()) + " token(s), 0 erros.");
        } catch (const SyntaxError& error) {
            output << "=== ERRO SINTATICO ===\n";
            output << "Linha " << error.getLine()
                   << ", Coluna " << error.getColumn()
                   << " - " << error.what() << "\n";
            output << "\nDica: Verifique a gramatica da linguagem. ";
            output << "Erros comuns incluem: ';' faltando, parenteses/chaves nao fechados, ";
            output << "expressao incompleta ou comando fora de ordem.\n";
            setStatus("Erro sintatico na linha " + std::to_string(error.getLine()) +
                      ", coluna " + std::to_string(error.getColumn()));
        }

        setControlText(gOutput, output.str());
    } catch (const std::exception& exception) {
        showError(owner, exception.what());
    }
}

// Abre um dialogo de selecao de arquivo e carrega o conteudo no editor.
void openFile(HWND owner) {
    char fileName[MAX_PATH] = "";
    OPENFILENAMEA options = {};
    options.lStructSize = sizeof(options);
    options.hwndOwner = owner;
    options.lpstrFile = fileName;
    options.nMaxFile = MAX_PATH;
    options.lpstrFilter = "GachaScript (*.gs)\0*.gs\0Arquivos texto\0*.txt\0Todos os arquivos\0*.*\0";
    options.nFilterIndex = 1;
    options.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&options)) {
        try {
            setControlText(gEditor, readTextFile(fileName));
            setStatus(std::string("Script carregado: ") + fileName);
        } catch (const std::exception& exception) {
            showError(owner, exception.what());
        }
    }
}

// Abre um dialogo de salvar e grava o conteudo da area de saida em arquivo .txt.
void saveOutput(HWND owner) {
    char fileName[MAX_PATH] = "drop_lexico.txt";
    OPENFILENAMEA options = {};
    options.lStructSize = sizeof(options);
    options.hwndOwner = owner;
    options.lpstrFile = fileName;
    options.nMaxFile = MAX_PATH;
    options.lpstrFilter = "Arquivo texto\0*.txt\0Todos os arquivos\0*.*\0";
    options.nFilterIndex = 1;
    options.lpstrDefExt = "txt";
    options.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&options)) {
        try {
            writeTextFile(fileName, getControlText(gOutput));
            setStatus(std::string("Drop salvo em: ") + fileName);
        } catch (const std::exception& exception) {
            showError(owner, exception.what());
        }
    }
}

void fillRectWithColor(HDC deviceContext, const RECT& area, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(deviceContext, &area, brush);
    DeleteObject(brush);
}

// Desenha botoes em estilo escuro/dourado em vez do botao padrao do Windows.
void drawThemedButton(const DRAWITEMSTRUCT* item) {
    const bool pressed = (item->itemState & ODS_SELECTED) != 0;
    const bool focused = (item->itemState & ODS_FOCUS) != 0;
    const bool primary = item->CtlID == ID_ANALYZE;
    const bool danger = item->CtlID == ID_CLEAR;

    RECT rect = item->rcItem;
    COLORREF fill = pressed ? COLOR_BUTTON_DOWN : COLOR_BUTTON;
    COLORREF border = COLOR_GOLD_DARK;
    COLORREF text = COLOR_TEXT;

    if (primary) {
        fill = pressed ? COLOR_GOLD_DARK : COLOR_GOLD;
        border = COLOR_GOLD;
        text = RGB(18, 20, 30);
    } else if (danger) {
        border = COLOR_RED;
    } else if ((item->itemState & ODS_HOTLIGHT) != 0) {
        fill = COLOR_BUTTON_HOT;
    }

    fillRectWithColor(item->hDC, rect, fill);

    HPEN borderPen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ previousPen = SelectObject(item->hDC, borderPen);
    HGDIOBJ previousBrush = SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
    Rectangle(item->hDC, rect.left, rect.top, rect.right, rect.bottom);
    SelectObject(item->hDC, previousBrush);
    SelectObject(item->hDC, previousPen);
    DeleteObject(borderPen);

    char label[80] = "";
    GetWindowTextA(item->hwndItem, label, sizeof(label));
    if (pressed) {
        OffsetRect(&rect, 1, 1);
    }

    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, text);
    HGDIOBJ previousFont = SelectObject(item->hDC, gUiFont);
    DrawTextA(item->hDC, label, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(item->hDC, previousFont);

    if (focused) {
        InflateRect(&rect, -4, -4);
        DrawFocusRect(item->hDC, &rect);
    }
}

// Recalcula posicao e tamanho de todos os controles ao redimensionar a janela.
void layoutControls(HWND window) {
    RECT area;
    GetClientRect(window, &area);

    const int width = area.right - area.left;
    const int height = area.bottom - area.top;
    const int margin = 18;
    const int headerTop = 12;
    const int titleHeight = 32;
    const int subtitleHeight = 24;
    const int toolbarTop = headerTop + titleHeight + subtitleHeight + 14;
    const int toolbarHeight = 34;
    const int labelHeight = 26;
    const int statusHeight = 30;
    const int gap = 14;
    const int buttonHeight = 30;

    MoveWindow(gHeaderTitle, margin, headerTop, width - (2 * margin), titleHeight, TRUE);
    MoveWindow(gHeaderSubtitle, margin, headerTop + titleHeight, width - (2 * margin), subtitleHeight, TRUE);

    int buttonLeft = margin;
    MoveWindow(GetDlgItem(window, ID_OPEN), buttonLeft, toolbarTop, 112, buttonHeight, TRUE);
    buttonLeft += 120;
    MoveWindow(GetDlgItem(window, ID_ANALYZE), buttonLeft, toolbarTop, 136, buttonHeight, TRUE);
    buttonLeft += 144;
    MoveWindow(GetDlgItem(window, ID_SAMPLE), buttonLeft, toolbarTop, 126, buttonHeight, TRUE);
    buttonLeft += 134;
    MoveWindow(GetDlgItem(window, ID_CLEAR), buttonLeft, toolbarTop, 92, buttonHeight, TRUE);
    buttonLeft += 100;
    MoveWindow(GetDlgItem(window, ID_SAVE), buttonLeft, toolbarTop, 122, buttonHeight, TRUE);
    buttonLeft += 130;
    MoveWindow(GetDlgItem(window, ID_EXIT), buttonLeft, toolbarTop, 72, buttonHeight, TRUE);

    const int labelsTop = toolbarTop + toolbarHeight + 12;
    const int contentTop = labelsTop + labelHeight;
    const int contentHeight = height - contentTop - statusHeight - margin;
    const int paneWidth = (width - (2 * margin) - gap) / 2;

    MoveWindow(gEditorLabel, margin, labelsTop, paneWidth, labelHeight, TRUE);
    MoveWindow(gOutputLabel, margin + paneWidth + gap, labelsTop, paneWidth, labelHeight, TRUE);

    MoveWindow(gEditor, margin, contentTop, paneWidth, contentHeight, TRUE);
    MoveWindow(gOutput, margin + paneWidth + gap, contentTop, paneWidth, contentHeight, TRUE);
    MoveWindow(gStatus, margin, height - statusHeight - 6, width - (2 * margin), statusHeight, TRUE);
}

// Cria todos os controles da janela (botoes, areas de texto, rotulos, fontes).
void createControls(HWND window, HINSTANCE instance) {
    createThemeResources();

    gTitleFont = CreateFontA(-26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    gUiFont = CreateFontA(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                          DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    gSmallFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    gMonoFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            FIXED_PITCH | FF_MODERN, "Consolas");

    gHeaderTitle = CreateWindowExA(0, "STATIC", APP_TITLE, WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_TITLE), instance, nullptr);
    gHeaderSubtitle = CreateWindowExA(0, "STATIC", APP_SUBTITLE, WS_CHILD | WS_VISIBLE,
                                      0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_SUBTITLE), instance, nullptr);

    CreateWindowExA(0, "BUTTON", "Abrir .gs", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_OPEN), instance, nullptr);
    CreateWindowExA(0, "BUTTON", ANALYZE_BUTTON_TEXT, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_ANALYZE), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Carregar demo", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_SAMPLE), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Limpar", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_CLEAR), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Salvar drop", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_SAVE), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Sair", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_EXIT), instance, nullptr);

    gEditorLabel = CreateWindowExA(0, "STATIC", "Script .gs", WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 0, window, nullptr, instance, nullptr);
    gOutputLabel = CreateWindowExA(0, "STATIC", "Tokens, drops e erros", WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 0, window, nullptr, instance, nullptr);

    gEditor = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                              WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                                  ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
                              0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_EDITOR), instance, nullptr);

    gOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                              WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                                  ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                              0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_OUTPUT), instance, nullptr);

    gStatus = CreateWindowExA(0, "STATIC", "Pronto para analisar GachaScript.", WS_CHILD | WS_VISIBLE,
                              0, 0, 0, 0, window, nullptr, instance, nullptr);

    SendMessageA(gHeaderTitle, WM_SETFONT, reinterpret_cast<WPARAM>(gTitleFont), TRUE);
    SendMessageA(gHeaderSubtitle, WM_SETFONT, reinterpret_cast<WPARAM>(gSmallFont), TRUE);
    SendMessageA(gEditor, WM_SETFONT, reinterpret_cast<WPARAM>(gMonoFont), TRUE);
    SendMessageA(gOutput, WM_SETFONT, reinterpret_cast<WPARAM>(gMonoFont), TRUE);
    SendMessageA(gEditorLabel, WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);
    SendMessageA(gOutputLabel, WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);
    SendMessageA(gStatus, WM_SETFONT, reinterpret_cast<WPARAM>(gSmallFont), TRUE);
    SendMessageA(gEditor, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 10));
    SendMessageA(gOutput, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 10));

    for (int id : {ID_OPEN, ID_ANALYZE, ID_SAMPLE, ID_CLEAR, ID_SAVE, ID_EXIT}) {
        SendMessageA(GetDlgItem(window, id), WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);
    }

    setControlText(gEditor, SAMPLE_CODE);
}

HBRUSH colorStaticControl(HDC deviceContext, HWND control) {
    SetBkMode(deviceContext, TRANSPARENT);

    if (control == gHeaderTitle) {
        SetTextColor(deviceContext, COLOR_GOLD);
        return gBackgroundBrush;
    }
    if (control == gHeaderSubtitle) {
        SetTextColor(deviceContext, COLOR_MUTED);
        return gBackgroundBrush;
    }
    if (control == gEditorLabel) {
        SetTextColor(deviceContext, COLOR_GREEN);
        return gBackgroundBrush;
    }
    if (control == gOutputLabel) {
        SetTextColor(deviceContext, COLOR_GOLD);
        return gBackgroundBrush;
    }
    if (control == gStatus) {
        SetTextColor(deviceContext, COLOR_MUTED);
        SetBkColor(deviceContext, COLOR_STATUS);
        return gStatusBrush;
    }
    if (control == gOutput) {
        SetBkMode(deviceContext, OPAQUE);
        SetTextColor(deviceContext, COLOR_TEXT);
        SetBkColor(deviceContext, COLOR_OUTPUT);
        return gOutputBrush;
    }

    SetTextColor(deviceContext, COLOR_TEXT);
    return gBackgroundBrush;
}

HBRUSH colorEditControl(HDC deviceContext, HWND control) {
    SetBkMode(deviceContext, OPAQUE);

    if (control == gEditor) {
        SetTextColor(deviceContext, COLOR_TEXT);
        SetBkColor(deviceContext, COLOR_EDITOR);
        return gEditorBrush;
    }

    SetTextColor(deviceContext, COLOR_TEXT);
    SetBkColor(deviceContext, COLOR_OUTPUT);
    return gOutputBrush;
}

// Procedimento de janela: trata mensagens de criacao, layout, pintura e comandos.
LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            createControls(window, reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
            layoutControls(window);
            return 0;
        case WM_SIZE:
            layoutControls(window);
            InvalidateRect(window, nullptr, TRUE);
            return 0;
        case WM_ERASEBKGND: {
            RECT area;
            GetClientRect(window, &area);
            fillRectWithColor(reinterpret_cast<HDC>(wParam), area, THEME_BACKGROUND);
            return 1;
        }
        case WM_CTLCOLORSTATIC:
            return reinterpret_cast<INT_PTR>(colorStaticControl(reinterpret_cast<HDC>(wParam),
                                                               reinterpret_cast<HWND>(lParam)));
        case WM_CTLCOLOREDIT:
            return reinterpret_cast<INT_PTR>(colorEditControl(reinterpret_cast<HDC>(wParam),
                                                             reinterpret_cast<HWND>(lParam)));
        case WM_CTLCOLORBTN:
            SetBkColor(reinterpret_cast<HDC>(wParam), THEME_BACKGROUND);
            return reinterpret_cast<INT_PTR>(gPanelBrush);
        case WM_DRAWITEM:
            drawThemedButton(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_OPEN:
                    openFile(window);
                    return 0;
                case ID_ANALYZE:
                    analyzeSource(window);
                    return 0;
                case ID_SAMPLE:
                    setControlText(gEditor, SAMPLE_CODE);
                    SetWindowTextA(gOutput, "");
                    setStatus("Demo GachaScript carregada.");
                    return 0;
                case ID_CLEAR:
                    SetWindowTextA(gEditor, "");
                    SetWindowTextA(gOutput, "");
                    setStatus("Editor limpo. Pronto para novo script.");
                    return 0;
                case ID_SAVE:
                    saveOutput(window);
                    return 0;
                case ID_EXIT:
                    DestroyWindow(window);
                    return 0;
                default:
                    break;
            }
            break;
        case WM_DESTROY:
            destroyThemeResources();
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }

    return DefWindowProcA(window, message, wParam, lParam);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    createThemeResources();

    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = gBackgroundBrush;

    if (!RegisterClassA(&windowClass)) {
        MessageBoxA(nullptr, "Nao foi possivel registrar a janela.", "GachaScript Lexer", MB_ICONERROR | MB_OK);
        destroyThemeResources();
        return 1;
    }

    HWND window = CreateWindowExA(0, WINDOW_CLASS_NAME, WINDOW_TITLE,
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                  1180, 760, nullptr, nullptr, instance, nullptr);

    if (window == nullptr) {
        MessageBoxA(nullptr, "Nao foi possivel criar a janela.", "GachaScript Lexer", MB_ICONERROR | MB_OK);
        destroyThemeResources();
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message = {};
    while (GetMessageA(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return static_cast<int>(message.wParam);
}