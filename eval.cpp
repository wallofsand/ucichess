
#include "eval.h"
#include "piece_location_tables.h"
#include "chess.h"
#include "square.h"

int EV::eval(Board::board_type* bp) {
    int material_score = eval_material(bp);
    int mobility_score = eval_mobility(bp);
    int position_score = eval_position(bp);
    return material_score + mobility_score + position_score;
}

int EV::eval_material(Board::board_type* bp) {
    int score = 0;
    U64 b = bp->black;
    U64 w = bp->white;
    while (b) {
        score -= piece_value[bp->piece_bb(b&-b)];
        b &= b - 1; // Clear the LS1B
    }
    while (w) {
        score += piece_value[bp->piece_bb(w&-w)];
        w &= w - 1; // Clear the LS1B
    }
    // std::cout << "mat score: " << score << std::endl;
    return score;
}

int EV::eval_mobility(Board::board_type* bp) {
    return 0;
}

int EV::eval_position(Board::board_type* bp) {
    int score = 0;
    U64 b = bp->black;
    U64 w = bp->white;
    float weight = BB::count_bits(bp->occ) / 32;
    // std::cout << "weight: " << weight << std::endl;
    while (b) {
        int sq = BB::bit_scan_forward(b);
        float delta = PLT::complex_read(bp->piece_bb(b&-b), sq, weight, true);
        score -= delta;
        // std::cout << "piece: " << CH::piece_char[bp->piece_bb(b&-b)]
        //           << " square: " << SQ::string_from_square[sq]
        //           << " delta: " << delta << std::endl;
        b &= b - 1; // Clear the LS1B
    }
    while (w) {
        int sq = BB::bit_scan_forward(w);
        float delta = PLT::complex_read(bp->piece_bb(w&-w), sq, weight, false);
        score += delta;
        // std::cout << "piece: " << CH::piece_char[bp->piece_bb(w&-w)]
        //           << " square: " << SQ::string_from_square[sq]
        //           << " delta: " << delta << std::endl;
        w &= w - 1; // Clear the LS1B
    }
    return score;
}
