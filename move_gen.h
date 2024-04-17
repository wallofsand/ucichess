
#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "bitboard.h"
#include "move.h"
#include "board.h"

namespace MoveGen {
    // 128 moves per 30 ply of search = 3840
    extern Move::move32 search_moves_128x30[128*30];
    extern int search_ply;
    extern int search_index;
    extern bool check;
    extern bool double_check;
    extern U64 op_atk;
    extern U64 check_ray;
    extern U64 pins; // bitboard of active player's pinned pieces

    void clear_array(int start=0);
    void gen_op_attack_mask(Board::board_type* bp);
    void find_pins(Board::board_type* bp);
    void checks_exist(Board::board_type* bp);
    void gen_moves(Board::board_type* bp);
    void gen_pawn_moves(Board::board_type* bp);
    void pawn_promotions(Board::board_type* bp);
    void gen_knight_moves(Board::board_type* bp);
    void gen_bishop_moves(Board::board_type* bp);
    void gen_rook_moves(Board::board_type* bp);
    void gen_king_moves(Board::board_type* bp);
    void sort_moves(int ply);
    Board::board_type* make_move(Board::board_type* bp, Move::move32 mv);
    void perft_root(Board::board_type* bp, int d);
    U64 perft(Board::board_type* bp, int d);
} // namespace MoveGen

#endif
