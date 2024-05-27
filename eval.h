
#ifndef EVAL_H
#define EVAL_H

#include "board.h"


namespace EV {
    static int piece_value[6] = { 100, 320, 330, 500, 980, 20000 };
    int eval(Board::board_type* bp);
    int eval_material(Board::board_type* bp);
    int eval_mobility(Board::board_type* bp);
    int eval_position(Board::board_type* bp);
} // namespace EV


#endif
