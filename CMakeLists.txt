cmake_minimum_required(VERSION 3.27)
project(PA3)

set(CMAKE_CXX_STANDARD 17)

add_executable(PA3 main.cpp
        parserInterp.h
        lex.h
        val.h)
