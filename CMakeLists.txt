cmake_minimum_required(VERSION 4.1)
project(Myrmidon)

set(CMAKE_CXX_STANDARD 26)

add_executable(chess_engine main.cpp
        Position.cpp
        Position.h
        types.h
        Zobrist.cpp
        Zobrist.h
        Move.h
        Attacks.cpp
        Attacks.h
        MoveGen.cpp
        MoveGen.h
)

# required for _pext_u64
target_compile_options(chess_engine PRIVATE -mbmi2)