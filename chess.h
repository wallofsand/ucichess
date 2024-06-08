
#ifndef CHESS_H
#define CHESS_H

#include <string>

namespace CH {
    const int PAWN = 0;
    const int KNIGHT = 1;
    const int BISHOP = 2;
    const int ROOK = 3;
    const int QUEEN = 4;
    const int KING = 5;
    const int WHITE_INDEX = 0;
    const int BLACK_INDEX = 1;

    /*
     * Indexed by [6 * color + piece] where
     *      color: (0 .. 1) white, black
     *      piece: (0 .. 5) pawn .. king
     */
    static std::string piece_char[12] = {
        "P", "N", "B", "R", "Q", "K",
        "p", "n", "b", "r", "q", "k"
    };
    inline int char_to_piece(char ch) {
        return ch == 'P' || ch == 'p' ? PAWN :
               ch == 'N' || ch == 'n' ? KNIGHT :
               ch == 'B' || ch == 'b' ? BISHOP :
               ch == 'R' || ch == 'r' ? ROOK :
               ch == 'Q' || ch == 'q' ? QUEEN :
               ch == 'K' || ch == 'k' ? KING : -1;
    }
    inline bool char_to_color(char ch) {
        return (ch < 97) ? WHITE_INDEX : BLACK_INDEX;
    }
}

#endif
