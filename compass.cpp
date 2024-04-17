// begin compass.cpp

#include "compass.h"

U64 Compass::knight_attacks[64] = {};
U64 Compass::king_attacks[64] = {};
uint8_t Compass::edge_distance_64x8[64][8] = {};
uint8_t Compass::first_rank_attacks_64x8[64*8] = {};

// function to access rank attack array
U64 Compass::get_rank_attacks(U64 occ, int sq) {
    unsigned int file       = sq &  7;
    unsigned int rankX8     = sq & 56; // rank * 8
    unsigned int rank_occX2 = (occ >> rankX8) & 2*63;
    // BB::print_binary_string(BB::build_binary_string(rank_occX2), "rank_occX2");
    // return 8 * rank_occ + file shifted to the right rank
    U64 attacks = first_rank_attacks_64x8[4*rank_occX2 + file]; // 8 * rank occupancy + file
    // BB::print_binary_string(BB::build_binary_string(attacks), "attacks");
    return attacks << rankX8;
}

// Method to return a bitboard of the ray which
// passes through square sq in direction DIRS[dir_index]
U64 Compass::build_ray(int sq, int dir_index) {
    U64 ray = 0;
    for (int step = 1; step <= Compass::edge_distance_64x8[sq][dir_index]; step++) {
        ray |= 1ull << (sq + step * Dir::DIRS[dir_index]);
    }
    return ray;
}

// Method to return a bitboard of the ray which
// passes between two colinear squares sq0 to sq1
U64 Compass::build_ray(int sq[2]) {
    int end_index = Dir::end_idx(CH::QUEEN);
    for (int i = 0; i < end_index; i++) {
        int dir = Dir::DIRS[i];
        for (int step = 1; step <= edge_distance_64x8[sq[0]][i]; step++)
            if (sq[0] + step * Dir::DIRS[i] == sq[1]) {
                U64 ray = 0;
                while(sq[0] != sq[1]) {
                    ray |= 1ull << sq[0];
                    sq += Dir::DIRS[i];
                }
                return ray;
            }
    }
    return 0;
}

// Method to return a bitboard of the ray from squares
// start (exclusive) through square end to board edge
// retuns zero if no such ray exists
// an optional occupancy array can be passed to stop the ray
U64 Compass::ray_square(int start, int end, U64 occ) {
    U64 ray = BB::nort_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::NoEa_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::east_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::SoEa_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::sout_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::SoWe_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::west_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::NoWe_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    return 0ull;
}

// Method to return a bitboard of the ray from squares
// start (exclusive) through square end to board edge
// retuns zero if no such ray exists
// an optional occupancy array can be passed to stop the ray
U64 Compass::ray_bb(U64 start, U64 end, U64 occ) {
    U64 ray = BB::nort_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::NoEa_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::east_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::SoEa_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::sout_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::SoWe_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::west_attacks(start, ~occ);
    if (ray & end) return ray;
    ray = BB::NoWe_attacks(start, ~occ);
    if (ray & end) return ray;
    return 0ull;
}

// initializer function:
void Compass::compute_edge_distances() {
    for (int sq = 0; sq < 64; sq++) {
        uint8_t rank = rank_yindex(sq);
        uint8_t file = file_xindex(sq);
        uint8_t nstep = 7 - rank;
        uint8_t estep = 7 - file;
        uint8_t sstep = rank;
        uint8_t wstep = file;
        edge_distance_64x8[sq][0] = nstep;
        edge_distance_64x8[sq][1] = estep;
        edge_distance_64x8[sq][2] = sstep;
        edge_distance_64x8[sq][3] = wstep;
        edge_distance_64x8[sq][4] = nstep < estep ? nstep : estep;
        edge_distance_64x8[sq][5] = nstep < wstep ? nstep : wstep;
        edge_distance_64x8[sq][6] = sstep < estep ? sstep : estep;
        edge_distance_64x8[sq][7] = sstep < wstep ? sstep : wstep;
    }
}

// function to precompute moves of knights:
void Compass::compute_knight_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        knight_attacks[sq] = 0ull;
        if (edge_distance_64x8[sq][0] > 0) {
            if (edge_distance_64x8[sq][0] > 1 && edge_distance_64x8[sq][1] > 0)
                knight_attacks[sq] |= 1ull << (sq + Dir::NNE);
            if (edge_distance_64x8[sq][1] > 1)
                knight_attacks[sq] |= 1ull << (sq + Dir::NEE);
            if (edge_distance_64x8[sq][0] > 1 && edge_distance_64x8[sq][3] > 0)
                knight_attacks[sq] |= 1ull << (sq + Dir::NNW);
            if (edge_distance_64x8[sq][3] > 1)
                knight_attacks[sq] |= 1ull << (sq + Dir::NWW);
        }
        if (edge_distance_64x8[sq][2] > 0) {
            if (edge_distance_64x8[sq][2] > 1 && edge_distance_64x8[sq][1] > 0)
                knight_attacks[sq] |= 1ull << (sq + Dir::SSE);
            if (edge_distance_64x8[sq][1] > 1)
                knight_attacks[sq] |= 1ull << (sq + Dir::SEE);
            if (edge_distance_64x8[sq][2] > 1 && edge_distance_64x8[sq][3] > 0)
                knight_attacks[sq] |= 1ull << (sq + Dir::SSW);
            if (edge_distance_64x8[sq][3] > 1)
                knight_attacks[sq] |= 1ull << (sq + Dir::SWW);
        }
    }
}

// function to precompute rank moves for rook and queen
void Compass::compute_rank_attacks() {
    for (U64 occ = 0; occ < 128; occ+=2) {
        for (int rksq = 0; rksq < 8; rksq++) {
            U64 rook = 1ull << rksq;
            first_rank_attacks_64x8[4*occ+rksq]  = BB::east_attacks(rook, ~occ) & 255;
            first_rank_attacks_64x8[4*occ+rksq] |= BB::west_attacks(rook, ~occ) & 255;
            first_rank_attacks_64x8[4*occ+rksq] &= ~rook;
        }
    }
}

// function to precompute king moves from each square
void Compass::compute_king_attacks() {
    U64 moves = (0b111<<16 | 0b101<<8 | 0b111) << 9;
    int shift = -18;
    for (int sq = 0; sq < 64; sq++) {
        king_attacks[sq] = BB::gen_shift(moves, shift + sq);
        king_attacks[sq] &= Compass::file_xindex(sq)==7 ? BB::NOT_A_FILE :
                            Compass::file_xindex(sq)==0 ? BB::NOT_H_FILE :
                            ~0ull;
    }
}
