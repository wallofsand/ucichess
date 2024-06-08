
#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <string>
#include "chess.h"
#include "square.h"

namespace Move {
    /*
     * bits: 0baaa_bbb_ccc_dddddd_eeeeee_fghi
     * - a: captured piece
     * - b: bitwise inverse of moving piece
     * - c: moving piece
     * - d: end square
     * - e: start square
     * - f: promo flag
     * - g: capture flag
     * - h: ep flag and promote type
     * - i: 2nd bit for promote type
     */
    typedef uint32_t move32;

    namespace Flag {
        const uint8_t PAWN_DOUBLE_ADVANCE    = 0b0001;
        const uint8_t CASTLE_KINGSIDE        = 0b0010;
        const uint8_t CASTLE_QUEENSIDE       = 0b0011;
        const uint8_t CAPTURE                = 0b0100;
        const uint8_t EN_PASSANT             = 0b0101;
        const uint8_t PROMOTION              = 0b1000;
        const uint8_t PROMOTE_KNIGHT         = 0b1000;
        const uint8_t PROMOTE_BISHOP         = 0b1001;
        const uint8_t PROMOTE_ROOK           = 0b1010;
        const uint8_t PROMOTE_QUEEN          = 0b1011;
        const uint8_t CAPTURE_PROMOTE_KNIGHT = 0b1100;
        const uint8_t CAPTURE_PROMOTE_BISHOP = 0b1101;
        const uint8_t CAPTURE_PROMOTE_ROOK   = 0b1110;
        const uint8_t CAPTURE_PROMOTE_QUEEN  = 0b1111;
        inline std::string to_string(int f) {
            return f == PAWN_DOUBLE_ADVANCE    ? "PAWN_DOUBLE_ADVANCE" :
                   f == CASTLE_KINGSIDE        ? "CASTLE_KINGSIDE" :
                   f == CASTLE_QUEENSIDE       ? "CASTLE_QUEENSIDE" :
                   f == CAPTURE                ? "CAPTURE" :
                   f == EN_PASSANT             ? "EN_PASSANT" :
                   f == PROMOTE_KNIGHT         ? "PROMOTE_KNIGHT" :
                   f == PROMOTE_BISHOP         ? "PROMOTE_BISHOP" :
                   f == PROMOTE_ROOK           ? "PROMOTE_ROOK" :
                   f == PROMOTE_QUEEN          ? "PROMOTE_QUEEN" :
                   f == CAPTURE_PROMOTE_KNIGHT ? "CAPTURE_PROMOTE_KNIGHT" :
                   f == CAPTURE_PROMOTE_BISHOP ? "CAPTURE_PROMOTE_BISHOP" :
                   f == CAPTURE_PROMOTE_ROOK   ? "CAPTURE_PROMOTE_ROOK" :
                   f == CAPTURE_PROMOTE_QUEEN  ? "CAPTURE_PROMOTE_QUEEN" : "NONE";
        }
    } // namespace Flag

    inline move32 build_move(int flag, int start, int end, int piece, int target) {
        return (flag & Flag::CAPTURE ? (target+1) & 7 : 0) << 22 | (~piece&7) << 19 | piece << 16 | end << 10 | start << 4 | flag;
    }

    inline std::string name(move32 mv) {
        std::string start = SQ::string_from_square[mv>> 4&63];
        std::string end   = SQ::string_from_square[mv>>10&63];
        int flag = mv & Flag::PROMOTE_QUEEN;
        std::string promo = flag == Flag::PROMOTE_KNIGHT ? "n" :
                            flag == Flag::PROMOTE_BISHOP ? "b" :
                            flag == Flag::PROMOTE_ROOK   ? "r" :
                            flag == Flag::PROMOTE_QUEEN  ? "q" :
                            "";
        return start + end + promo;
    }

    inline std::string to_string(move32 mv) {
        std::string start  = ", Start: " + SQ::string_from_square[mv>>4&63];
        std::string end    = ", End: " + SQ::string_from_square[mv>>10&63];
        std::string piece  = ", Piece: " + CH::piece_char[mv>>16&7];
        std::string flag   = ", Flag: " + Flag::to_string(mv&15);
        // TODO: Delete and redo:
        // std::string target = (mv & Flag::CAPTURE) ? ", Target: " + (mv%15 == Flag::EN_PASSANT ? CH::piece_char[0] : CH::piece_char[(mv>>22&7)-1]) : "";
        return "Move: " + std::to_string(mv) + start + end + piece + flag; // target + flag;
    }

} // namespace Move

#endif
