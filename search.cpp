
#include "search.h"
#include "move_gen.h"

using namespace MoveGen;

Move::move32 Search::search(Board::board_type* bp, int depth) {
    result r = root_nega_max(bp, depth, -99999, 99999);
    return r.move;
}

Search::result Search::root_nega_max(Board::board_type* bp, int depth, int alpha, int beta) {
    result r;
    search_ply = 0;
    clear_array();
    gen_moves(bp);
    sort_moves(search_ply);
    search_ply++;
    Move::move32 mv = 0;
    int max = -99999;
    Board::board_type* next;
    for (int idx = 0; idx < 128 && search_moves_128x30[idx+(search_ply-1<<7)]; idx++) {
        std::cout << Move::name(search_moves_128x30[idx+(search_ply-1<<7)]) << ": ";
        next = make_move(bp, search_moves_128x30[idx+(search_ply-1<<7)]);
        int score = -nega_max(next, depth-1, -beta, -alpha);
        std::cout << score << std::endl;
        mv = score > max ? search_moves_128x30[idx+(search_ply-1<<7)] : mv;
        max = score > max ? score : max;
    }
    r.score = max;
    r.move = mv;
    return r;
}

int Search::nega_max(Board::board_type* bp, int depth, int alpha, int beta) {
    const int side_to_move[2] = { 1, -1 };
    if (depth == 0) {
        return EV::eval(bp) * side_to_move[bp->black_to_move];
    }
    int max = -99999;
    gen_moves(bp);
    sort_moves(search_ply);
    search_ply++;
    Board::board_type* next = new Board::board_type();
    for (int idx = 0; idx < 128 && search_moves_128x30[idx+(search_ply-1<<7)]; idx++) {
        next = make_move(bp, search_moves_128x30[idx+(search_ply-1<<7)]);
        int score = -nega_max(next, depth-1, -beta, -alpha);
        max = score > max ? score : max;
    }
    delete next;
    search_ply--;
    clear_array(search_ply << 7);
    return max;
}
