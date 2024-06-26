// Copied from my original cppChess engine
#ifndef PIECE_LOCATION_TABLES_H
#define PIECE_LOCATION_TABLES_H


namespace PLT {
    const int read(const int table[64], int sq, bool is_black);
    const float complex_read(int type, int sq, float middlegame_weight, bool is_black);

    extern const int* middlegame_piece_tables[6];
    extern const int* endgame_piece_tables[6];

    const int pawns[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        40, 40, 40, 40, 40, 40, 40, 40,
        30, 30, 30, 30, 30, 30, 30, 30,
        0, 0, 0, 25, 25, 0, 0, 0,
        0, -10, -10, 15, 15, -10, -10, 0,
        -5, 20, 20, -20, -20, 20, 20, -5,
        0, 0, 0, 0, 0, 0, 0, 0 };
    const int knights[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -30, -20, -20, -10, -10, -20, -20, -30,
        -20, 0, 20, 30, 30, 20, 0, -20,
        -10, 0, 40, 50, 50, 40, 0, -10,
        -10, 0, 40, 50, 50, 40, 0, -10,
        -20, 0, 25, 20, 20, 25, 0, -20,
        -30, -20, -20, -10, -10, -20, -20, -30,
        -50, -35, -20, -20, -20, -20, -35, -50 };
    const int bishops[64] = {
        -40, -20, -10, -10, -10, -10, -20, -40,
        0, 0, 10, 10, 10, 10, 0, 0,
        0, 10, 20, 20, 20, 20, 10, 0,
        0, 10, 30, 50, 50, 30, 10, 0,
        0, 10, 40, 50, 50, 40, 10, 0,
        0, 10, 20, 40, 40, 20, 10, 0,
        5, 30, 5, 20, 20, 5, 30, 5,
        -50, -20, -5, -10, -10, -5, -20, -50 };
    const int rooks[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 10, 10, 10, 0, 0, -5 };
    const int queens[64] = {
        -5, -5, -5, -5, -5, -5, -5, -5,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0 };
    // two tables for kings: middlegame and endgame
    const int kings_middle[64] = {
        -50, -50, -50, -50, -50, -50, -50, -50,
        -40, -40, -40, -40, -40, -40, -40, -40,
        -40, -40, -40, -40, -40, -40, -40, -40,
        -30, -30, -30, -50, -50, -40, -30, -20,
        -30, -30, -40, -50, -50, -40, -30, -30,
        -30, -30, -30, -30, -30, -30, -30, -30,
        -30, -30, -35, -35, -35, -35, -30, -30,
        0, 0, 40, -30, -25, -30, 40, 0 };
    const int kings_end[64] = {
        -50, -20, -10, -10, -10, -10, -20, -50,
        -50, 0, 10, 10, 10, 10, 0, -50,
        -50, 0, 10, 15, 15, 10, 0, -50,
        -50, 0, 15, 25, 25, 15, 0, -50,
        -50, 0, 15, 25, 25, 15, 0, -50,
        -50, 0, 10, 15, 15, 10, 0, -50,
        -50, -20, -5, -5, -5, -5, -20, -50,
        -50, -30, -10, -10, -10, -10, -30, -50 };
}

#endif
