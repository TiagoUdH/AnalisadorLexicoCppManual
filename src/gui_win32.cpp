#include "Lexer.h"

#include <stdexcept>
#include <string>
#include <windows.h>
#include <commdlg.h>

namespace {

constexpr int ID_EDITOR = 1001;
constexpr int ID_OUTPUT = 1002;
constexpr int ID_OPEN = 1003;
constexpr int ID_ANALYZE = 1004;
constexpr int ID_SAMPLE = 1005;
constexpr int ID_CLEAR = 1006;
constexpr int ID_SAVE = 1007;

HWND gEditor = nullptr;
HWND gOutput = nullptr;
HWND gStatus = nullptr;
HWND gEditorLabel = nullptr;
HWND gOutputLabel = nullptr;
HFONT gUiFont = nullptr;
HFONT gMonoFont = nullptr;

const char* SAMPLE_CODE =
    "Start {\r\n"
    "    Level ataque = 10;\r\n"
    "    T4 chance = 50;\r\n"
    "    T5 dano = 90;\r\n"
    "    Meta ativo = 1;\r\n"
    "    Lore nome = \"texto\";\r\n"
    "    Assinatura fixo = 1;\r\n"
    "    Anunciar(nome);\r\n"
    "    Coletar(ataque);\r\n"
    "    50/50 (ataque >= 10 E ativo == 1) {\r\n"
    "        Drop ataque;\r\n"
    "    } Garantido {\r\n"
    "        Farmar(i = 0; i < 10; i = i + 1) {\r\n"
    "            Combo(i < 5 OU ataque != 0) {\r\n"
    "                Seletor(i) {\r\n"
    "                    Quebra_Fraqueza;\r\n"
    "                }\r\n"
    "            }\r\n"
    "        }\r\n"
    "    }\r\n"
    "}\r\n"
    "j@\r\n"
    "1a\r\n"
    "2.a3\r\n"
    "5555555555555555\r\n"
    "minha_variavel_para_testar_um_nome_muito_longo = 1\r\n"
    "texto_ruim = \"hello world\r\n"
    "char_ruim = 'a\r\n"
    "@\r\n"
    "/* comentario sem fechamento";

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

std::string getControlText(HWND handle) {
    const int length = GetWindowTextLengthA(handle);
    std::string text(static_cast<std::size_t>(length) + 1, '\0');

    if (length > 0) {
        GetWindowTextA(handle, text.data(), length + 1);
    }

    text.resize(static_cast<std::size_t>(length));
    return text;
}

void setControlText(HWND handle, const std::string& text) {
    const std::string converted = toWindowsNewlines(text);
    SetWindowTextA(handle, converted.c_str());
}

void setStatus(const std::string& text) {
    if (gStatus != nullptr) {
        SetWindowTextA(gStatus, text.c_str());
    }
}

void showError(HWND owner, const std::string& message) {
    MessageBoxA(owner, message.c_str(), "Analisador Lexico", MB_ICONERROR | MB_OK);
}

void analyzeSource(HWND owner) {
    try {
        Lexer lexer(getControlText(gEditor));
        const LexicalResult result = lexer.analyze();
        setControlText(gOutput, Lexer::formatResult(result));

        setStatus("Analise concluida: " + std::to_string(result.tokens.size()) +
                  " token(s), " + std::to_string(result.errors.size()) + " erro(s).");
    } catch (const std::exception& exception) {
        showError(owner, exception.what());
    }
}

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
            setStatus(std::string("Arquivo carregado: ") + fileName);
        } catch (const std::exception& exception) {
            showError(owner, exception.what());
        }
    }
}

void saveOutput(HWND owner) {
    char fileName[MAX_PATH] = "saida_lexica.txt";
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
            setStatus(std::string("Saida salva em: ") + fileName);
        } catch (const std::exception& exception) {
            showError(owner, exception.what());
        }
    }
}

void layoutControls(HWND window) {
    RECT area;
    GetClientRect(window, &area);

    const int width = area.right - area.left;
    const int height = area.bottom - area.top;
    const int margin = 14;
    const int toolbarHeight = 38;
    const int labelHeight = 24;
    const int statusHeight = 26;
    const int gap = 12;
    const int buttonWidth = 96;
    const int buttonHeight = 28;
    const int top = margin;

    int buttonLeft = margin;
    MoveWindow(GetDlgItem(window, ID_OPEN), buttonLeft, top, buttonWidth, buttonHeight, TRUE);
    buttonLeft += buttonWidth + 8;
    MoveWindow(GetDlgItem(window, ID_ANALYZE), buttonLeft, top, buttonWidth, buttonHeight, TRUE);
    buttonLeft += buttonWidth + 8;
    MoveWindow(GetDlgItem(window, ID_SAMPLE), buttonLeft, top, buttonWidth, buttonHeight, TRUE);
    buttonLeft += buttonWidth + 8;
    MoveWindow(GetDlgItem(window, ID_CLEAR), buttonLeft, top, buttonWidth, buttonHeight, TRUE);
    buttonLeft += buttonWidth + 8;
    MoveWindow(GetDlgItem(window, ID_SAVE), buttonLeft, top, buttonWidth + 18, buttonHeight, TRUE);

    const int contentTop = margin + toolbarHeight + labelHeight;
    const int contentHeight = height - contentTop - statusHeight - margin;
    const int paneWidth = (width - (2 * margin) - gap) / 2;

    MoveWindow(gEditorLabel, margin, margin + toolbarHeight, paneWidth, labelHeight, TRUE);
    MoveWindow(gOutputLabel, margin + paneWidth + gap, margin + toolbarHeight, paneWidth, labelHeight, TRUE);

    MoveWindow(gEditor, margin, contentTop, paneWidth, contentHeight, TRUE);
    MoveWindow(gOutput, margin + paneWidth + gap, contentTop, paneWidth, contentHeight, TRUE);
    MoveWindow(gStatus, margin, height - statusHeight - 4, width - (2 * margin), statusHeight, TRUE);
}

void createControls(HWND window, HINSTANCE instance) {
    gUiFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                          DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    gMonoFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            FIXED_PITCH | FF_MODERN, "Consolas");

    CreateWindowExA(0, "BUTTON", "Abrir", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_OPEN), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Analisar", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_ANALYZE), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Exemplo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_SAMPLE), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Limpar", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_CLEAR), instance, nullptr);
    CreateWindowExA(0, "BUTTON", "Salvar saida", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_SAVE), instance, nullptr);

    gEditorLabel = CreateWindowExA(0, "STATIC", "Codigo-fonte", WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 0, window, nullptr, instance, nullptr);
    gOutputLabel = CreateWindowExA(0, "STATIC", "Tokens e erros", WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 0, window, nullptr, instance, nullptr);

    gEditor = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                              WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                                  ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
                              0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_EDITOR), instance, nullptr);

    gOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                              WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                                  ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                              0, 0, 0, 0, window, reinterpret_cast<HMENU>(ID_OUTPUT), instance, nullptr);

    gStatus = CreateWindowExA(0, "STATIC", "Pronto.", WS_CHILD | WS_VISIBLE,
                              0, 0, 0, 0, window, nullptr, instance, nullptr);

    SendMessageA(gEditor, WM_SETFONT, reinterpret_cast<WPARAM>(gMonoFont), TRUE);
    SendMessageA(gOutput, WM_SETFONT, reinterpret_cast<WPARAM>(gMonoFont), TRUE);
    SendMessageA(gEditorLabel, WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);
    SendMessageA(gOutputLabel, WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);
    SendMessageA(gStatus, WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);

    for (int id : {ID_OPEN, ID_ANALYZE, ID_SAMPLE, ID_CLEAR, ID_SAVE}) {
        SendMessageA(GetDlgItem(window, id), WM_SETFONT, reinterpret_cast<WPARAM>(gUiFont), TRUE);
    }

    setControlText(gEditor, SAMPLE_CODE);
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            createControls(window, reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
            layoutControls(window);
            return 0;
        case WM_SIZE:
            layoutControls(window);
            return 0;
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
                    setStatus("Exemplo carregado.");
                    return 0;
                case ID_CLEAR:
                    SetWindowTextA(gEditor, "");
                    SetWindowTextA(gOutput, "");
                    setStatus("Editor limpo.");
                    return 0;
                case ID_SAVE:
                    saveOutput(window);
                    return 0;
                default:
                    break;
            }
            break;
        case WM_DESTROY:
            if (gUiFont != nullptr) {
                DeleteObject(gUiFont);
            }
            if (gMonoFont != nullptr) {
                DeleteObject(gMonoFont);
            }
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }

    return DefWindowProcA(window, message, wParam, lParam);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    const char* className = "AnalisadorLexicoManualCppWindow";

    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassA(&windowClass)) {
        MessageBoxA(nullptr, "Nao foi possivel registrar a janela.", "Analisador Lexico", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND window = CreateWindowExA(0, className, "Analisador Lexico Manual - C++",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                  1120, 720, nullptr, nullptr, instance, nullptr);

    if (window == nullptr) {
        MessageBoxA(nullptr, "Nao foi possivel criar a janela.", "Analisador Lexico", MB_ICONERROR | MB_OK);
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