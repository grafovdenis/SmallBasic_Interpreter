cmake_minimum_required(VERSION 3.10)
project(SmallBasic_Interpreter C)

set(CMAKE_C_STANDARD 11)

add_executable(SmallBasic_Interpreter main/main.c main/lexemes.h main/analyzer.c main/main.h main/variables.h main/variables.c main/io.c main/io.h main/branching.c main/branching.h)