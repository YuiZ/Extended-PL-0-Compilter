cmake_minimum_required(VERSION 3.7)
project(Extended_PL_0_Compilter)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        lib/pass_first.h
        lib/pass_second.h
        lib/yui_tools.h
        src/tools.cpp
        src/ErrorHandler.cpp
        src/GeneralPurposeRegisters.cpp
        src/LexiconAnalysis.cpp
        src/ObjectCodeProducer.cpp
        src/Optimizer.cpp
        src/SemanticAnalysis.cpp
        src/SymbolTable.cpp
        src/SyntaxAnalysis.cpp
        src/main.cpp)

add_executable(Extended_PL_0_Compilter ${SOURCE_FILES})