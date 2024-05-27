
#ifndef SEARCH_H
#define SEARCH_H

#include "eval.h"
#include "move.h"

namespace Search {
    struct result {
        int score;
        Move::move32 move;
        enum FLAG {
            FAIL_HIGH,
            FAIL_LOW,
            EXACT,
        } flag;
    };

    static int piece_value[6] = { 100, 320, 330, 500, 980, 20000 };
    Move::move32 search(Board::board_type* bp, int depth, int alpha, int beta);
    result root_nega_max(Board::board_type* bp, int depth, int alpha, int beta);
    int nega_max(Board::board_type* bp, int depth, int alpha, int beta);
} // namespace Search


#endif
