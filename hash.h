
#ifndef HASH_H
#define HASH_H

#include <random>

#include "board.h"
#include "move.h"
#include "square.h"
#include "compass.h"

namespace Hash {

    // smaller table cleared every move
    static const int DEFAULT_SIZE = 5595979;

    static const U64 DEFAULT_SEED = 15375420585056461361ull;

    // array of random bitstrings for each piece at each square
    static U64 sq_color_piece_64x2x6[64][2][6];

    // Kingside: 0, Queenside: 1
    static U64 castle_rights_wb_kq[2][2];

    // file 0 - 7 of ep square. 8 for no ep
    static U64 ep_file[9];

    static U64 black_to_move;

    static void init_zobrist_bits() {
        // the Mersenne Twister with a popular choice of parameters
        static std::mt19937_64 rng;
        static std::uniform_int_distribution<U64> U64_dist;

        rng.seed(DEFAULT_SEED);

        // position bitstrings
        for (int sq = 0; sq < 64; sq++) {
            for (int piece = 0; piece < 6; piece++) {
                sq_color_piece_64x2x6[sq][0][piece] = U64_dist(rng);
                sq_color_piece_64x2x6[sq][1][piece] = U64_dist(rng);
            }
        }

        // castle right bitstrings
        castle_rights_wb_kq[0][0] = U64_dist(rng);
        castle_rights_wb_kq[0][1] = U64_dist(rng);
        castle_rights_wb_kq[1][0] = U64_dist(rng);
        castle_rights_wb_kq[1][1] = U64_dist(rng);

        // en passant file bitstrings
        for (int file = 0; file < 8; file++) {
            ep_file[file] = U64_dist(rng);
        }
        ep_file[8] = 0ull;

        black_to_move = U64_dist(rng);

    }

    static void zobrist_test() {
        // write a method to record our zobrist bitstrings
        // method should compare bitstrings to some checksum
        // also implement a test set to evaluate the hash function
        return;
    }

    // hash the position from scratch
    static U64 hash(Board::board_type* bp) {
        U64 h = black_to_move * bp->black_to_move; // record the turn player
        U64 pieces = bp->occ; // pieces left to hash
        while (pieces) { // hash the pieces on the board
            int sq = BB::bit_scan_forward(pieces);
            h ^= sq_color_piece_64x2x6[sq][Board::color_at(bp, sq)][Board::piece_at(bp, sq)];
            pieces &= pieces - 1; // clear LS1B
        }
        h ^= ep_file[bp->ep_file]; // en passant
        h ^= castle_rights_wb_kq[0][0] * (bp->castle_qkQK      & 1); // white kingside castle
        h ^= castle_rights_wb_kq[0][1] * (bp->castle_qkQK >> 1 & 1); // white queenside castle
        h ^= castle_rights_wb_kq[1][0] * (bp->castle_qkQK >> 2 & 1); // black kingside castle
        h ^= castle_rights_wb_kq[1][1] * (bp->castle_qkQK >> 3 & 1); // black queenside castle
        return h;
    }

    // compute the hash if move mv was played on board bp
    static U64 update_hash(Board::board_type* bp, Move::move32 mv) {
        SQ::square_type end = SQ::square_type(mv>>10&63);
        // change the turn player flag
        U64 h = bp->zhash ^ black_to_move;
        // remove the moving piece:
        h ^= sq_color_piece_64x2x6[mv>>4&63][bp->black_to_move][mv>>16&7];
        // compute the moving piece in case of promotion:
        // int piece = mv&Move::Flag::PROMOTE_KNIGHT?(mv&3)+1:mv>>16&7;
        // replace the moving piece:
        h ^= sq_color_piece_64x2x6[end][bp->black_to_move][mv&Move::Flag::PROMOTE_KNIGHT?(mv&3)+1:mv>>16&7];
        // remove the captured piece:
        h ^= mv&15 == Move::Flag::CAPTURE ?
                sq_color_piece_64x2x6[end][!bp->black_to_move][mv>>19] : 0;
        // compute the rook start square for castling kingside:
        // int rksq_ks = bp->black_to_move ? SQ::h8 : h1;
        // compute the rook start square for castling queenside:
        // int rksq_qs = bp->black_to_move ? SQ::a8 : a1;
        // was it a castle? remove the rook:
        h ^= mv&15 == Move::Flag::CASTLE_KINGSIDE ?
                sq_color_piece_64x2x6[bp->black_to_move?SQ::h8:SQ::h1][bp->black_to_move][CH::ROOK] :
             mv&15 == Move::Flag::CASTLE_QUEENSIDE ?
                sq_color_piece_64x2x6[bp->black_to_move?SQ::h8:SQ::h1][bp->black_to_move][CH::ROOK] : 0;
        // compute the rook end square for castling kingside:
        // int endsq_ks = bp->black_to_move ? SQ::f8 : SQ::f1;
        // compute the rook end square for castling queenside:
        // int endsq_qs = bp->black_to_move ? SQ::d8 : SQ::d1;
        // was it a castle? replace the rook:
        h ^= mv&15 == Move::Flag::CASTLE_KINGSIDE ?
                sq_color_piece_64x2x6[bp->black_to_move?SQ::f8:SQ::d1][bp->black_to_move][CH::ROOK] :
             mv&15 == Move::Flag::CASTLE_QUEENSIDE ?
                sq_color_piece_64x2x6[bp->black_to_move?SQ::d8:SQ::f1][bp->black_to_move][CH::ROOK] : 0;
        // handle en passant capture:
        h ^= mv&15 == Move::Flag::EN_PASSANT ?
            sq_color_piece_64x2x6[end-Dir::PAWN_DIR[bp->black_to_move]][!bp->black_to_move][CH::PAWN] : 0;
        // reset the ep file:
        h ^= ep_file[bp->ep_file];
        // set a new ep file on pawn double move
        h ^= mv&15 == Move::Flag::PAWN_DOUBLE_ADVANCE ? ep_file[end&7] : 0;
        return h;
    }

}; // namespace Hash

#endif
