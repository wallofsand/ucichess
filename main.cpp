
#include <iostream>
#include <iomanip>
#include "chess.h"
#include "bitboard.h"
#include "compass.h"
#include "board.h"
#include "hash.h"
#include "eval.h"
#include "piece_location_tables.h"
#include "square.h"
#include "move_gen.h"

struct stack_node {
    Board::board_type* bp = nullptr;
    stack_node* prev      = nullptr;
    ~stack_node() {
        delete prev;
        delete bp;
    }
};

int main (int args, char** argv) {
    // precompute board data
    Compass::compute_edge_distances();
    Compass::compute_knight_attacks();
    Compass::compute_rank_attacks();
    Compass::compute_king_attacks();

    stack_node* top = new stack_node;
    top->bp = Board::new_game();
    // Board::board_type* board = Board::new_game();

    // MoveGen::perft_root(top->bp, 2);

    // MoveGen::gen_moves(top->bp);
    // MoveGen::sort_moves(0);
    // for (int idx = 0; idx < 128 && MoveGen::search_moves_128x30[idx]; idx++) {
    //     std::cout << std::setfill('_') << std::setw(3) << idx;
    //     std::cout << ": " << Move::to_string(MoveGen::search_moves_128x30[idx]) << std::endl;
    // }

    bool running = true;
    while (running) {
        int perft_depth;
        MoveGen::clear_array();
        MoveGen::search_ply = 0;
        MoveGen::gen_moves(top->bp);
        MoveGen::sort_moves(MoveGen::search_ply);

        Board::print_board(top->bp, CH::WHITE_INDEX);
        for (int idx = 0; idx < MoveGen::search_index; idx++) {
            Move::move32 mv = MoveGen::search_moves_128x30[idx];
            std::cout << Move::name(mv) << " ";
            if (idx != MoveGen::search_index-1 && idx % 5 == 4) std::cout << std::endl;
        }
        std::cout << std::endl << "Choose a move: ";

        std::string instr;
        std::cin >> instr;
        for (int idx = 0; idx < MoveGen::search_index; idx++) {
            Move::move32 mv = MoveGen::search_moves_128x30[idx];
            std::string mvstr = SQ::string_from_square[mv>>4&63] + SQ::string_from_square[mv>>10&63];
            if (instr == mvstr) {
                stack_node* next = new stack_node;
                next->bp = MoveGen::make_move(top->bp, mv);
                next->prev = top;
                top = next;
            }
        }
        if (instr == "undo" && top->prev) {
            stack_node* tmp = top;
            top = top->prev;
            tmp->prev = nullptr;
            delete tmp;
        } else if (instr == "perft") {
            perft_depth = -1;
            while (perft_depth < 0) {
                std::cin >> perft_depth;
            }
            MoveGen::perft_root(top->bp, perft_depth);
        } else if (instr == "mask") {
            MoveGen::gen_op_attack_mask(top->bp);
            BB::print_binary_string(BB::build_binary_string(MoveGen::op_atk), "op_atk");
        } else if (instr == "bb") {
            Board::print_bitboards(top->bp);
        } else if (instr == "end" || instr == "stop" || instr == "quit") running = false;
    }

    return 0;
}