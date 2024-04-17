cmake_minimum_required(VERSION 3.0.0)
project(uciChess VERSION 0.0.0)

include_directories(uciChess ${CMAKE_CURRENT_SOURCE_DIR})

set(DCMAKE_CXX_FLAGS -pg)
set(DCMAKE_EXE_LINKER_FLAGS -pg)
set(DCMAKE_SHARED_LINKER_FLAGS -pg)

add_executable(uciChess
    main.cpp
    bitboard.cpp
    board.cpp
    compass.cpp
    eval.cpp
    move_gen.cpp
    piece_location_tables.cpp)
