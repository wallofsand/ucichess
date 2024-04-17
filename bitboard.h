// Copied from my original cppChess engine

#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <string>
#include <iostream>

typedef std::uint64_t U64;

namespace BB {
    // intersect before west shifts
    const U64 NOT_A_FILE = 0xfefefefefefefefe;
    // intersect before east shifts
    const U64 NOT_H_FILE = 0x7f7f7f7f7f7f7f7f;
    const U64 A_FILE = 0x0101010101010101;
    const U64 ROW_1 = 0xffull;
    const U64 ROW_2 = ROW_1<<8;
    const U64 ROW_3 = ROW_2<<8;
    const U64 ROW_4 = ROW_3<<8;
    const U64 ROW_5 = ROW_4<<8;
    const U64 ROW_6 = ROW_5<<8;
    const U64 ROW_7 = ROW_6<<8;
    const U64 ROW_8 = ROW_7<<8;
    const U64 deBruijnMagic = 0x02cdeae5187e2769; // 26586981
    const unsigned int deBruijnSequence[64] = {
        0,  1,  2, 37, 47,  3, 32, 38,
        61, 48, 27,  4, 33, 10, 51, 39,
        62, 30, 59, 49, 28, 19,  5, 21,
        34,  7, 56, 11, 23, 52, 14, 40,
        63, 36, 46, 31, 60, 26,  9, 50,
        29, 58, 18, 20,  6, 55, 22, 13,
        35, 45, 25,  8, 57, 17, 54, 12,
        44, 24, 16, 53, 43, 15, 42, 41,
    };

    // Functions:
    unsigned int bit_scan_forward (U64 bb);
    U64 nort_shift_one(U64 bb);
    U64 sout_shift_one(U64 bb);
    U64 east_shift_one(U64 bb);
    U64 west_shift_one(U64 bb);
    U64 NoEa_shift_one(U64 bb);
    U64 NoWe_shift_one(U64 bb);
    U64 SoEa_shift_one(U64 bb);
    U64 SoWe_shift_one(U64 bb);
    U64 nort_occl_fill(U64 gen, U64 empty);
    U64 sout_occl_fill(U64 gen, U64 empty);
    U64 east_occl_fill(U64 gen, U64 empty);
    U64 west_occl_fill(U64 gen, U64 empty);
    U64 NoEa_occl_fill(U64 gen, U64 empty);
    U64 NoWe_occl_fill(U64 gen, U64 empty);
    U64 SoEa_occl_fill(U64 gen, U64 empty);
    U64 SoWe_occl_fill(U64 gen, U64 empty);
    U64 nort_attacks(U64 rooks, U64 empty);
    U64 sout_attacks(U64 rooks, U64 empty);
    U64 east_attacks(U64 rooks, U64 empty);
    U64 west_attacks(U64 rooks, U64 empty);
    U64 NoEa_attacks(U64 bishops, U64 empty);
    U64 NoWe_attacks(U64 bishops, U64 empty);
    U64 SoEa_attacks(U64 bishops, U64 empty);
    U64 SoWe_attacks(U64 bishops, U64 empty);
    U64 gen_shift(U64 bb, int s);
    U64 flip_vertical(U64 x);
    U64 flip_diag_A1H8(U64 x);
    U64 rotate_clockwise(U64 x);
    bool contains_square(U64 bb, int sq);
    int count_bits(U64 bb);
    // void print_U64(U64 bb, std::string name = "", bool fmt = false);
    void print_binary_string(std::string bbstr, std::string name="");
    std::string build_binary_string(U64 bb);
    int lz_count(U64 bb);
}

#endif
