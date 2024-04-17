// Copied from my original cppChess engine
#include "piece_location_tables.h"


const int* PLT::middlegame_piece_tables[7] = 
{
    nullptr, PLT::pawns, PLT::knights,
    PLT::bishops, PLT::rooks,
    PLT::queens, PLT::kings_middle
};

const int* PLT::endgame_piece_tables[7] =
{
    nullptr, PLT::pawns, PLT::knights,
    PLT::bishops, PLT::rooks,
    PLT::queens, PLT::kings_end
};

// all tables are read from black's perspective
// when white reads a table, we need to flip it along the Y axis
const int PLT::read(const int table[64], int sq, bool is_black)
{
    sq = is_black ? sq : ((7 - (sq >> 3)) << 3) + (sq % 8);
    return table[sq];
}

// method to interpolate normal and endgame tables
// middlegameWeight is the number of pieces / 32 - a value between 0.0 and 1.0
const float PLT::complex_read(int type, int sq, float middlegame_weight, bool is_black)
{
    float opening = read(middlegame_piece_tables[type], sq, is_black) * middlegame_weight;
    float endgame = read(endgame_piece_tables[type], sq, is_black) * (1 - middlegame_weight);
    return opening + endgame;
}
