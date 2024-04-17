
#ifndef COMPASS_H
#define COMPASS_H

#include "bitboard.h"
#include "chess.h"

// consts for bitboard offsets per direction
namespace Dir {
    const int NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
            NORTHEAST = 9, NORTHWEST = 7, SOUTHEAST = -7, SOUTHWEST = -9,
            NNE = 17, NEE = 10, SEE = -6, SSE = -15,
            SSW = -17, SWW = -10, NWW = 6, NNW = 15;
    const int DIRS[16] = { NORTH, EAST, SOUTH, WEST,
            NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST,
            NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW};
    const int PAWN_DIR[2] = { NORTH, SOUTH };
    // functions to compute array indicies
    inline int start_idx(int piece) {
        switch (piece) {
            case CH::KNIGHT: return 8;
            case CH::BISHOP: return 4;
            default:         return 0;
        }
    }
    inline int end_idx(int piece) {
        switch (piece) {
            case CH::KNIGHT: return 16;
            case CH::ROOK:   return 4;
            default:         return 8;
        }
    }
} // namespace Dir

// namespace to store some functions for search
// and precomputed data about some piece moves
namespace Compass {
    extern U64 knight_attacks[64];
    extern U64 king_attacks[64];
    extern uint8_t edge_distance_64x8[64][8];
    extern uint8_t first_rank_attacks_64x8[64*8];
    U64 get_rank_attacks(U64 occ, int sq);
    U64 build_ray(int sq, int dir_index);
    U64 build_ray(int sq[2]);
    U64 ray_square(int start, int end, U64 occ = 0ull);
    U64 ray_bb(U64 start, U64 end, U64 occ = 0ull);
    // get the rank (horizontal row) of square sq
    inline int rank_yindex(int sq) {
        return sq / 8;
    }
    // get the file (vertical row) of square sq
    inline int file_xindex(int sq) {
        return sq % 8;
    }
    // initializer functions:
    void compute_edge_distances();
    void compute_knight_attacks();
    void compute_rank_attacks();
    void compute_king_attacks();
} // namespace Compass

#endif
