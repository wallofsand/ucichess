// Mostly copied from my original cppChess engine
#include "bitboard.h"

U64 BB::sq_bb[64] = {};
void BB::init_sq_bb() {
    for (int i = 0; i < 64; i++) {
        sq_bb[i] = 1ull << i;
    }
}

/**
 * bit_scan_forward
 * "Using de Bruijn Squences to locate the least significant one bit."
 * Created from https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
 * @author Gerd Isenberg
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
unsigned int BB::bit_scan_forward (U64 bb) {
    return deBruijnSequence[((bb & -bb) * deBruijnMagic) >> 58];
}

/**
 * contains_square
 * @param bb any bitboard
 * @param sq square index 0-63
 * @return true if the bit at sq is set
 */
bool BB::contains_square(U64 bb, int sq) {
    return bb & sq_bb[sq];
}

/**
 * count_bits
 * "Method to count the number of true bits in a bitboard."
 * @param bb any bitboard
 * @return the number of bits set in bb
 */
int BB::count_bits(U64 bb) {
    int count = 0;
    while (bb) {
        count++;
        // clear the LS1B
        bb &= bb - 1;
    }
    return count;
}

/**
 * north shift by 1
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::nort_shift_one(U64 bb) {
    return bb << 8;
}

/**
 * south shift by 1
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::sout_shift_one(U64 bb) {
    return bb >> 8;
}

/**
 * east shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::east_shift_one(U64 bb) {
    return (bb & NOT_H_FILE) << 1;
}

/**
 * west shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::west_shift_one(U64 bb) {
    return (bb & NOT_A_FILE) >> 1;
}

/**
 * northeast shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::NoEa_shift_one(U64 bb) {
    return (bb & NOT_H_FILE) << 9;
}

/**
 * northwest shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::NoWe_shift_one(U64 bb) {
    return (bb & NOT_A_FILE) << 7;
}

/**
 * southeast shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::SoEa_shift_one(U64 bb) {
    return (bb & NOT_H_FILE) >> 7;
}

/**
 * southwest shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 BB::SoWe_shift_one(U64 bb) {
    return (bb & NOT_A_FILE) >> 9;
}

/**
 * north occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::nort_occl_fill(U64 gen, U64 empty) {
    gen  |= empty & (gen   << 8);
    empty = empty & (empty << 8);
    gen  |= empty & (gen   << 16);
    empty = empty & (empty << 16);
    gen  |= empty & (gen   << 32);
    return gen;
}

/**
 * south occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty bitboard of empty squares
 * @return filled bitboard
 */
U64 BB::sout_occl_fill(U64 gen, U64 empty) {
    gen  |= empty & (gen   >> 8);
    empty = empty & (empty >> 8);
    gen  |= empty & (gen   >> 16);
    empty = empty & (empty >> 16);
    gen  |= empty & (gen   >> 32);
    return gen;
}

/**
 * east occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::east_occl_fill(U64 gen, U64 empty) {
    empty &= NOT_A_FILE; // inverted mask b/c of empty set
    gen   |= empty & (gen   << 1);
    empty  = empty & (empty << 1);
    gen   |= empty & (gen   << 2);
    empty  = empty & (empty << 2);
    gen   |= empty & (gen   << 4);
    return gen;
}

/**
 * west occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::west_occl_fill(U64 gen, U64 empty) {
    empty &= NOT_H_FILE; // inverted mask b/c of empty set
    gen   |= empty & (gen   >> 1);
    empty  = empty & (empty >> 1);
    gen   |= empty & (gen   >> 2);
    empty  = empty & (empty >> 2);
    gen   |= empty & (gen   >> 4);
    return gen;
}

/**
 * northeast occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::NoEa_occl_fill(U64 gen, U64 empty) {
    empty &= NOT_A_FILE; // inverted mask b/c of empty set
    gen   |= empty & (gen   << 9);
    empty  = empty & (empty << 9);
    gen   |= empty & (gen   << 18);
    empty  = empty & (empty << 18);
    gen   |= empty & (gen   << 36);
    return gen;
}

/**
 * northwest occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::NoWe_occl_fill(U64 gen, U64 empty) {
    empty &= NOT_H_FILE; // inverted mask b/c of empty set
    gen   |= empty & (gen   << 7);
    empty  = empty & (empty << 7);
    gen   |= empty & (gen   << 14);
    empty  = empty & (empty << 14);
    gen   |= empty & (gen   << 28);
    return gen;
}

/**
 * southeast occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::SoEa_occl_fill(U64 gen, U64 empty) {
    empty &= NOT_A_FILE; // inverted mask b/c of empty set
    gen   |= empty & (gen   >> 7);
    empty  = empty & (empty >> 7);
    gen   |= empty & (gen   >> 14);
    empty  = empty & (empty >> 14);
    gen   |= empty & (gen   >> 28);
    return gen;
}

/**
 * southwest occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return filled bitboard
 */
U64 BB::SoWe_occl_fill(U64 gen, U64 empty) {
    empty &= NOT_H_FILE; // inverted mask b/c of empty set
    gen   |= empty & (gen   >> 9);
    empty  = empty & (empty >> 9);
    gen   |= empty & (gen   >> 18);
    empty  = empty & (empty >> 18);
    gen   |= empty & (gen   >> 36);
    return gen;
}

U64 BB::nort_attacks(U64 bb, U64 empty) { return nort_shift_one(nort_occl_fill(bb, empty)); }
U64 BB::sout_attacks(U64 bb, U64 empty) { return sout_shift_one(sout_occl_fill(bb, empty)); }
U64 BB::east_attacks(U64 bb, U64 empty) { return east_shift_one(east_occl_fill(bb, empty)); }
U64 BB::west_attacks(U64 bb, U64 empty) { return west_shift_one(west_occl_fill(bb, empty)); }
U64 BB::NoEa_attacks(U64 bb, U64 empty) { return NoEa_shift_one(NoEa_occl_fill(bb, empty)); }
U64 BB::NoWe_attacks(U64 bb, U64 empty) { return NoWe_shift_one(NoWe_occl_fill(bb, empty)); }
U64 BB::SoEa_attacks(U64 bb, U64 empty) { return SoEa_shift_one(SoEa_occl_fill(bb, empty)); }
U64 BB::SoWe_attacks(U64 bb, U64 empty) { return SoWe_shift_one(SoWe_occl_fill(bb, empty)); }

/**
 * generalized shift
 * @author Gerd Isenberg
 * @param x any bitboard
 * @param s shift amount -64 < s < +64
 *          left if positive
 *          right if negative
 * @return shifted bitboard
 */
U64 BB::gen_shift(U64 bb, int s) {
    char left = (char) s;
    char right = -((char) (s >> 8) & left);
    return (bb >> right) << (right + left);
}

/**
 * Method to print a uint64_t
 * @param bb any bitboard
 * @param fmt true if a pretty frame should print
 */
// void BB::print_U64(U64 bb, std::string name, bool fmt) {
//     if (name != "") std::cout << name << ":" << std::endl;
//     BB::print_binary_string(BB::build_binary_string(bb), fmt);
// }

/**
 * method to build string representation of bitboards
 * Little-Endian, a1 bit is the first char of the string
 * @param bb any bitboard
 * @return a string of the binary representation of bb
 */
std::string BB::build_binary_string(U64 bb) {
    int leading_zero_count = lz_count(bb);
    std::string zeros = "";
    for (int i = 0; i < leading_zero_count; i++)
        zeros.append("0");
    while (bb) {
        zeros.insert(leading_zero_count, std::to_string(bb & 1));
        bb = bb >> 1;
    }
    // string is currently reversed - the string is the correct binary representation of the number
    // but that means the bits are indexed backwards. lets fix that
    for (int idx = 0; idx < 32; idx++) {
        char tmp = zeros[idx];
        zeros[idx] = zeros[63-idx];
        zeros[63-idx] = tmp;
    }
    return zeros;
}

/**
 * Flip a bitboard vertically about the centre ranks.
 * Rank 1 is mapped to rank 8 and vice versa.
 * @param x any bitboard
 * @return bitboard x flipped vertically
 */
U64 BB::flip_vertical(U64 x) {
    return _byteswap_uint64(x);
}

/**
 * Flip a bitboard about the diagonal a1-h8.
 * Square h1 is mapped to a8 and vice versa.
 * @param x any bitboard
 * @return bitboard x flipped about diagonal a1-h8
 */
U64 BB::flip_diag_A1H8(U64 x) {
   U64 t;
   const U64 k1 = 0x5500550055005500;
   const U64 k2 = 0x3333000033330000;
   const U64 k4 = 0x0f0f0f0f00000000;
   t  = k4 & (x ^ (x << 28));
   x ^=       t ^ (t >> 28) ;
   t  = k2 & (x ^ (x << 14));
   x ^=       t ^ (t >> 14) ;
   t  = k1 & (x ^ (x <<  7));
   x ^=       t ^ (t >>  7) ;
   return x;
}

/**
 * Rotate a bitboard by 90 degrees clockwise.
 * @param x any bitboard
 * @return bitboard x rotated 90 degrees clockwise
 */
U64 BB::rotate_clockwise(U64 x) {
    return flip_vertical(flip_diag_A1H8(x));
}

/** Rotate a bitboard by 90 degrees counterclockwise.
 * @param x any bitboard
 * @return bitboard x rotated 90 degress counterclockwise
 */
U64 BB::rotate_counterclockwise(U64 x) {
    return flip_diag_A1H8(flip_vertical(x));
}

/**
 * method to print a 64 character string as a chessboard
 * Little-Endian, a1 is the first char of the string
 * @param bbstr any string of length 64
 * @param fmt true to print fancy output
 */
// void BB::print_binary_string(std::string bbstr, bool fmt) {
//     // we want to print the binary as 8 8-bit words
//     // the words are printed forewards but in reverse order
//     // since the printing happens top-to-bottom on the screen
//     std::string divider = "|---|---|---|---|---|---|---|---|\n";
//     for (int file = 7; file >= 0; file--) {
//         fmt::print("{}", fmt ? divider + "| " : "");
//         for (int rank = 0; rank < 8; rank++)
//             fmt::print("{}{}", bbstr[(file<<3) + rank], fmt ? " | " : "");
//         fmt::print("\n");
//     }
//     fmt::print("{}", fmt ? divider : "");
// }

void BB::print_binary_string(std::string bbstr, std::string name) {
    if (name != "") {
        std::cout << " " << name << ":" <<std::endl;
    }
    for (int file = 7; file >= 0; file--) {
        for (int rank = 0; rank < 8; rank++) {
            std::cout << bbstr[(file<<3) + rank];
        }
        std::cout << std::endl;
    }
}

/**
 * method to count the number of leading zeros in a bitboard
 * @param any bitboard
 * @return the number of leading zeros
 */
int BB::lz_count(U64 bb) {
    if (!bb) return 64;
    int count = 0;
    // is the msb in the top 32 bits?
    if (!(bb & 0xFFFFFFFF00000000)) {
        // it wasn't, shift 32 up
        count += 32;
        bb = bb << 32;
    }
    // how about the next 16?
    if (!(bb & 0xFFFF000000000000)) {
        count += 16;
        bb = bb << 16;
    }
    // how about the next 8?
    if (!(bb & 0xFF00000000000000)) {
        count += 8;
        bb = bb << 8;
    }
    if (!(bb & 0xF000000000000000)) {
        count += 4;
        bb = bb << 4;
    }
    if (!(bb & 0xC000000000000000)) {
        count += 2;
        bb = bb << 2;
    }
    if (!(bb & 0x8000000000000000)) {
        count += 1;
    }
    return count;
}
