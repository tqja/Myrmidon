cmake_minimum_required(VERSION 4.1)
project(Myrmidon)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(Myrmidon main.cpp
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
        perft.h
)

# required for _pext_u64
target_compile_options(Myrmidon PRIVATE -mbmi2)
