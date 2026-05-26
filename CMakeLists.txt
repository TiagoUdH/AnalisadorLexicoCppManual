cmake_minimum_required(VERSION 3.15)

project(AnalisadorLexicoManualCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lexer
    src/Lexer.cpp
)

target_include_directories(lexer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(analisador_cli
    src/main_cli.cpp
)

target_link_libraries(analisador_cli PRIVATE lexer)

if(WIN32)
    add_executable(analisador_gui WIN32
        src/gui_win32.cpp
    )

    target_link_libraries(analisador_gui PRIVATE lexer comdlg32)
endif()