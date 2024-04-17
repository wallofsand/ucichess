
#ifndef BOARD_H
#define BOARD_H

#include <string>
#include "bitboard.h"

namespace Board {
    struct board_type {
        U64 occ             = 0;
        U64 white           = 0;
        U64 black           = 0;
        U64 pawns           = 0;
        U64 knights         = 0;
        U64 bishops         = 0;
        U64 rooks           = 0;
        U64 queens          = 0;
        U64 kings           = 0;
        U64 zhash           = 0;
        uint8_t ep_file     = 8; // 8 to store no en passant allowed
        uint8_t castle_qkQK = 0b0000;
        bool black_to_move  = false;
        uint32_t halfmoves  = 0;
        uint32_t fullmoves  = 1;
        U64* bb_piece[6] = { &pawns, &knights, &bishops, &rooks, &queens, &kings };
        U64* bb_color[2] = { &white, &black };
    };
    board_type* new_game();
    board_type* copy(board_type* bp);
    board_type* build_fen(std::string fen);
    bool square_occupied(board_type* bp, int sq);
    bool color_at(board_type* bp, int sq);
    int piece_at(board_type* bp, int sq);
    int piece_bb(board_type* bp, U64 bb);
    void print_board(board_type* bp, bool black_to_move = false);
    void print_bitboards(board_type* bp);

    const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}; // namespace Board

#endif
