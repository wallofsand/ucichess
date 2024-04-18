
#include <iomanip>
#include "move_gen.h"
#include "compass.h"


Move::move32 MoveGen::search_moves_128x30[128*30] {};
int MoveGen::search_ply {};
int MoveGen::search_index {};
bool MoveGen::check {};
bool MoveGen::double_check {};
U64 MoveGen::op_atk {};
U64 MoveGen::check_ray {};
U64 MoveGen::pins {};

void MoveGen::clear_array(int start) {
    for (int idx = start; idx < 128*30; idx++) {
        search_moves_128x30[idx] = 0;
    }
}

/**
 * gen_attack_mask
 * Method to get the opponents' attack mask
 * @return the bitboard of squares attacked by opponent's pieces
 */
void MoveGen::gen_op_attack_mask(Board::board_type* bp) {
    U64 op = *bp->bb_color[!bp->black_to_move];
    U64 empty = ~bp->occ | (bp->kings & *bp->bb_color[bp->black_to_move]);
    op_atk = Compass::king_attacks[BB::bit_scan_forward(bp->kings&op)];

    // pawn attacks
    U64 attackers = bp->pawns & op;
    U64 pattacksWest = bp->black_to_move ? BB::NoWe_shift_one(bp->pawns & op) : BB::SoWe_shift_one(bp->pawns & op);
    U64 pattacksEast = bp->black_to_move ? BB::NoEa_shift_one(bp->pawns & op) : BB::SoEa_shift_one(bp->pawns & op);
    op_atk |= pattacksEast;
    op_atk |= pattacksWest;

    // rooks & queens
    attackers = (bp->rooks | bp->queens) & op;
    op_atk |= BB::nort_attacks(attackers, empty);
    op_atk |= BB::sout_attacks(attackers, empty);
    op_atk |= BB::east_attacks(attackers, empty);
    op_atk |= BB::west_attacks(attackers, empty);

    // bishops & queens
    attackers = (bp->bishops | bp->queens) & op;
    op_atk |= BB::NoEa_attacks(attackers, empty);
    op_atk |= BB::NoWe_attacks(attackers, empty);
    op_atk |= BB::SoEa_attacks(attackers, empty);
    op_atk |= BB::SoWe_attacks(attackers, empty);

    // knights
    attackers = bp->knights & op;
    while (attackers) {
        // knight attacks from knight at LS1B
        op_atk |= Compass::knight_attacks[BB::bit_scan_forward(attackers)];
        // clear the LS1B
        attackers &= attackers - 1;
    }
}

// setup method to get pins in a position
// modifies a bitboard of squares containing pinning pieces
void MoveGen::find_pins(Board::board_type* bp) {
    // active king
    U64 king = bp->kings & *bp->bb_color[bp->black_to_move];
    // enemy sliding pieces
    U64 empty = ~*bp->bb_color[!bp->black_to_move];
    U64 op_rooks = (bp->rooks | bp->queens) & *bp->bb_color[!bp->black_to_move];
    U64 op_bishops = (bp->bishops | bp->queens) & *bp->bb_color[!bp->black_to_move];
    // North
    U64 ray = BB::nort_attacks(king, empty);
    pins = (ray & op_rooks && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // East
    ray = BB::east_attacks(king, empty);
    pins |= (ray & op_rooks && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // South
    ray = BB::sout_attacks(king, empty);
    pins |= (ray & op_rooks && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // West
    ray = BB::west_attacks(king, empty);
    pins |= (ray & op_rooks && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // NorthEast
    ray = BB::NoEa_attacks(king, empty);
    pins |= (ray & op_bishops && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // NorthWest
    ray = BB::NoWe_attacks(king, empty);
    pins |= (ray & op_bishops && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // SouthEast
    ray = BB::SoEa_attacks(king, empty);
    pins |= (ray & op_bishops && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
    // SouthWest
    ray = BB::SoWe_attacks(king, empty);
    pins |= (ray & op_bishops && BB::count_bits(ray & *bp->bb_color[bp->black_to_move]) == 1)
            ? ray & *bp->bb_color[bp->black_to_move] : 0ull;
}

void MoveGen::checks_exist(Board::board_type* bp) {
    check = false;
    double_check = false;
    U64 king = bp->kings & *bp->bb_color[bp->black_to_move];

    if (!(king & op_atk)) // check only exists if king is attacked
        return;

    int ksq = BB::bit_scan_forward(king);
    U64 op  = *bp->bb_color[!bp->black_to_move];

    // knights
    check_ray = bp->knights & op & Compass::knight_attacks[ksq];
    check     = check_ray;

    // pawns
    U64 attackers  = BB::gen_shift(king & BB::NOT_H_FILE, Dir::PAWN_DIR[!bp->black_to_move] + 1);
    attackers     |= BB::gen_shift(king & BB::NOT_A_FILE, Dir::PAWN_DIR[!bp->black_to_move] - 1);
    attackers     &= op & bp->pawns;
    check_ray      = check_ray ? check_ray : attackers;
    check = check || attackers;

    // bishops, queens
    attackers  = BB::NoEa_attacks(king, ~bp->occ);
    attackers |= BB::NoWe_attacks(king, ~bp->occ);
    attackers |= BB::SoEa_attacks(king, ~bp->occ);
    attackers |= BB::SoWe_attacks(king, ~bp->occ);
    attackers &= op & (bp->queens | bp->bishops);
    if (attackers && !check_ray) {
        attackers = BB::NoEa_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->bishops) ? attackers : check_ray;
        attackers = BB::NoWe_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->bishops) ? attackers : check_ray;
        attackers = BB::SoEa_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->bishops) ? attackers : check_ray;
        attackers = BB::SoWe_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->bishops) ? attackers : check_ray;
    }
    double_check = check && attackers;
    check        = check || attackers;

    // rooks, queens
    attackers  = BB::nort_attacks(king, ~bp->occ);
    attackers |= BB::sout_attacks(king, ~bp->occ);
    attackers |= BB::east_attacks(king, ~bp->occ);
    attackers |= BB::west_attacks(king, ~bp->occ);
    attackers &= op & (bp->queens | bp->rooks);
    if (attackers && !check_ray) {
        attackers = BB::nort_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->rooks) ? attackers : check_ray;
        attackers = BB::sout_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->rooks) ? attackers : check_ray;
        attackers = BB::east_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->rooks) ? attackers : check_ray;
        attackers = BB::west_attacks(king, ~bp->occ);
        check_ray = attackers & op & (bp->queens | bp->rooks) ? attackers : check_ray;
    }
    double_check = check && attackers;
    check        = check || attackers;
}

void MoveGen::gen_moves(Board::board_type* bp) {
    search_index = (search_ply<<7);
    // setup functions:
    gen_op_attack_mask(bp);
    checks_exist(bp);
    find_pins(bp);
    // generate moves:
    gen_king_moves(bp);
    if (double_check) return;
    gen_rook_moves(bp);
    gen_bishop_moves(bp);
    gen_knight_moves(bp);
    gen_pawn_moves(bp);
}

// pawn moves, captures, promotions, and en passant
void MoveGen::gen_pawn_moves(Board::board_type* bp) {
    // pawn single and double advances
    U64 pawns = bp->pawns & *bp->bb_color[bp->black_to_move];
    U64 moves = BB::gen_shift(pawns, Dir::PAWN_DIR[bp->black_to_move]) & ~bp->occ;
    U64 doubles = BB::gen_shift(moves & (bp->black_to_move ? BB::ROW_6 : BB::ROW_3), Dir::PAWN_DIR[bp->black_to_move]) & ~bp->occ;
    U64 promotions = moves & (BB::ROW_1 | BB::ROW_8);
    moves ^= promotions;
    moves      &= check ? check_ray : ~0ull;
    doubles    &= check ? check_ray : ~0ull;
    promotions &= check ? check_ray : ~0ull;
    // serialize the doubles bitboard:
    while (doubles) {
        int endsq = BB::bit_scan_forward(doubles);
        int start = endsq - 2*Dir::PAWN_DIR[bp->black_to_move];
        // only pinned pawns in front of the king can advance
        if ((pins & BB::sq_bb[start]) && !(BB::A_FILE << (start&7) & bp->kings & *bp->bb_color[bp->black_to_move])) {
            // this move is illegal because of a pin
            doubles &= doubles - 1; // clear the LS1B
            continue;
        }
        search_moves_128x30[search_index] = Move::build_move(Move::Flag::PAWN_DOUBLE_ADVANCE, start, endsq, CH::PAWN, 0);
        search_index++;
        doubles &= doubles - 1; // clear the LS1B
    }
    // serialize the moves bitboard:
    while (moves) {
        int endsq = BB::bit_scan_forward(moves);
        int start = endsq - Dir::PAWN_DIR[bp->black_to_move];
        // only pinned pawns in front of the king can advance
        if ((pins & BB::sq_bb[start]) && !(BB::A_FILE << (start&7) & bp->kings & *bp->bb_color[bp->black_to_move])) {
            // this move is illegal because of a pin
            moves &= moves - 1; // clear the LS1B
            continue;
        }
        search_moves_128x30[search_index] = Move::build_move(0, start, endsq, CH::PAWN, 0);
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    // serialize the promotions bitboard:
    while (promotions) {
        int endsq = BB::bit_scan_forward(promotions);
        // pinned pawns cannot promote without capture
        int start = endsq - Dir::PAWN_DIR[bp->black_to_move];
        if (pins & BB::sq_bb[start]) {
            // this move is illegal because of a pin
            promotions &= promotions - 1; // clear the LS1B
            continue;
        }
        search_moves_128x30[search_index]   = Move::build_move(Move::Flag::PROMOTE_QUEEN, start, endsq, CH::PAWN, 0);
        search_moves_128x30[search_index+1] = Move::build_move(Move::Flag::PROMOTE_ROOK, start, endsq, CH::PAWN, 0);
        search_moves_128x30[search_index+2] = Move::build_move(Move::Flag::PROMOTE_BISHOP, start, endsq, CH::PAWN, 0);
        search_moves_128x30[search_index+3] = Move::build_move(Move::Flag::PROMOTE_KNIGHT, start, endsq, CH::PAWN, 0);
        search_index += 4;
        promotions &= promotions - 1; // clear the LS1B
    }
    // is there an ep capture available?
    U64 epsq = bp->ep_file < 8 ? (BB::A_FILE << bp->ep_file & (bp->black_to_move ? BB::ROW_3 : BB::ROW_6)) : 0ull;
    // is the ep pawn delivering check?
    U64 ep_ray = check_ray & BB::gen_shift(epsq, Dir::PAWN_DIR[!bp->black_to_move]) & bp->pawns & *bp->bb_color[!bp->black_to_move];
    // pawn captures east
    moves = BB::gen_shift(pawns & BB::NOT_H_FILE, Dir::DIRS[4+2*bp->black_to_move]) & (*bp->bb_color[!bp->black_to_move] | epsq);
    promotions = moves & (BB::ROW_1 | BB::ROW_8);
    moves ^= promotions;
    moves      &= check ? check_ray | ep_ray : ~0ull;
    promotions &= check ? check_ray          : ~0ull;
    while (moves) {
        int endsq = BB::bit_scan_forward(moves);
        int start = endsq - Dir::DIRS[4+2*bp->black_to_move];
        uint8_t flag = epsq & moves & -moves ? Move::Flag::EN_PASSANT : Move::Flag::CAPTURE;
        // check for pins
        int target = flag == Move::Flag::EN_PASSANT ? CH::PAWN : Board::piece_at(bp, endsq);
        search_moves_128x30[search_index] = Move::build_move(flag, start, endsq, CH::PAWN, target);
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    while (promotions) {
        int endsq = BB::bit_scan_forward(promotions);
        int start = endsq - Dir::DIRS[5+2*bp->black_to_move];
        int target = Board::piece_at(bp, endsq);
        search_moves_128x30[search_index]   = Move::build_move(Move::Flag::CAPTURE_PROMOTE_QUEEN,  start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+1] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_ROOK,   start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+2] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_BISHOP, start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+3] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_KNIGHT, start, endsq, CH::PAWN, target);
        search_index += 4;
        promotions &= promotions - 1; // clear the LS1B
    }
    // pawn captures west
    moves = BB::gen_shift(pawns & BB::NOT_A_FILE, Dir::DIRS[5+2*bp->black_to_move]) & (*bp->bb_color[!bp->black_to_move] | epsq);
    promotions = moves & (BB::ROW_1 | BB::ROW_8);
    moves ^= promotions;
    moves      &= check ? check_ray | ep_ray : ~0ull;
    promotions &= check ? check_ray          : ~0ull;
    while (moves) {
        int endsq = BB::bit_scan_forward(moves);
        int start = endsq - Dir::DIRS[5+2*bp->black_to_move];
        uint8_t flag = epsq & moves & -moves ? Move::Flag::EN_PASSANT : Move::Flag::CAPTURE;
        int target = flag == Move::Flag::EN_PASSANT ? CH::PAWN : Board::piece_at(bp, endsq);
        search_moves_128x30[search_index] = Move::build_move(flag, start, endsq, CH::PAWN, target);
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    while (promotions) {
        int endsq = BB::bit_scan_forward(promotions);
        int start = endsq - Dir::DIRS[5+2*bp->black_to_move];
        int target = Board::piece_at(bp, endsq);
        search_moves_128x30[search_index]   = Move::build_move(Move::Flag::CAPTURE_PROMOTE_QUEEN,  start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+1] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_ROOK,   start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+2] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_BISHOP, start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+3] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_KNIGHT, start, endsq, CH::PAWN, target);
        search_index += 4;
        promotions &= promotions - 1; // clear the LS1B
    }
}

void MoveGen::gen_knight_moves(Board::board_type* bp) {
    U64 knights = bp->knights & *bp->bb_color[bp->black_to_move];
    // pinned knights can never move
    knights &= ~pins;
    while (knights) {
        int start = BB::bit_scan_forward(knights);
        U64 moves = Compass::knight_attacks[start] & ~*bp->bb_color[bp->black_to_move];
        while (moves) {
            int end = BB::bit_scan_forward(moves);
            if (check && !(BB::sq_bb[end] & check_ray)) {
                // this move does not stop check
                moves &= moves - 1; // clear the LS1B
                continue;
            }
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            search_moves_128x30[search_index] = Move::build_move(
                flag, start, end, CH::KNIGHT, flag ? Board::piece_bb(bp, moves & -moves) : 0
            );
            search_index++;
            moves &= moves - 1; // clear the LS1B
        }
        knights &= knights - 1; // clear the LS1B
    }
}

void MoveGen::gen_bishop_moves(Board::board_type* bp) {
    U64 b = (bp->bishops | bp->queens) & *bp->bb_color[bp->black_to_move];
    while (b) {
        int start = BB::bit_scan_forward(b);
        U64 moves = BB::NoEa_attacks(b&-b, ~bp->occ) | BB::NoWe_attacks(b&-b, ~bp->occ)
                  | BB::SoEa_attacks(b&-b, ~bp->occ) | BB::SoWe_attacks(b&-b, ~bp->occ);
        moves &= ~*bp->bb_color[bp->black_to_move];
        moves &= check ? check_ray : ~0ull;
        while (moves) {
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            int end = BB::bit_scan_forward(moves);
            search_moves_128x30[search_index] = Move::build_move(
                flag, start, end, bp->bishops & b & -b ? CH::BISHOP : CH::QUEEN, flag ? Board::piece_at(bp, end) : 0
            );
            search_index++;
            moves &= moves - 1; // clear the LS1B
        }
        b &= b - 1; // clear the LS1B
    }
}

void MoveGen::gen_rook_moves(Board::board_type* bp) {
    U64 r    = (bp->rooks | bp->queens) & *bp->bb_color[bp->black_to_move];
    U64 rocc = BB::rotate_clockwise(bp->occ);
    U64 rcol = BB::rotate_clockwise(*bp->bb_color[bp->black_to_move]);
    while (r) {
        int start = BB::bit_scan_forward(r);
        int piece = bp->rooks & r & -r ? CH::ROOK : CH::QUEEN;
        // East/West moves
        U64 moves = Compass::get_rank_attacks(bp->occ, start) & ~*bp->bb_color[bp->black_to_move];
        moves &= check ? check_ray : ~0ull;
        while (moves) {
            int end = BB::bit_scan_forward(moves);
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            int target = flag ? Board::piece_at(bp, end) : 0;
            search_moves_128x30[search_index] = Move::build_move(flag, start, end, piece, target);
            search_index++;
            moves &= moves - 1; // clear the LS1B
        }
        int rsq = (((start >> 3) | (start << 3)) & 63) ^ 56;
        // North/South moves
        moves = Compass::get_rank_attacks(rocc, rsq) & ~rcol;
        moves = BB::rotate_counterclockwise(moves);
        moves &= check ? check_ray : ~0ull;
        while (moves) {
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            int end = BB::bit_scan_forward(moves);
            int target = flag ? Board::piece_at(bp, end) : 0;
            search_moves_128x30[search_index] = Move::build_move(flag, start, end, piece, target);
            search_index++;
            moves &= moves - 1; // clear the LS1B
        }
        r &= r - 1; // clear the LS1B
    }
}

void MoveGen::gen_king_moves(Board::board_type* bp) {
    U64 k = bp->kings & *bp->bb_color[bp->black_to_move];
    int ksq = BB::bit_scan_forward(k & -k);
    U64 moves = Compass::king_attacks[ksq] & ~op_atk & ~*bp->bb_color[bp->black_to_move];
    while (moves) {
        uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
        int end = BB::bit_scan_forward(moves & -moves);
        search_moves_128x30[search_index] = Move::build_move(
            flag, ksq, end, CH::KING, flag ? Board::piece_at(bp, end) : 0
        );
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    uint8_t qk = bp->castle_qkQK >> 2 * bp->black_to_move;
    const U64 KS_MASK = bp->black_to_move ? 112ull << 56 : 112; // 0b111 << 4 = 112
    const U64 QS_MASK = bp->black_to_move ? 28ull  << 56 : 28;  // 0b111 << 2 = 28 << 56
    search_moves_128x30[search_index] = qk&1 && (KS_MASK & (check_ray | bp->occ)) == 0 ? Move::build_move(
        Move::Flag::CASTLE_KINGSIDE, ksq, ksq+2, CH::KING, 0
    ) : 0;
    search_index += qk&1 && (KS_MASK & (check_ray | bp->occ)) == 0;
    search_moves_128x30[search_index] = qk&2 && (QS_MASK & (check_ray | bp->occ)) == 0 ? Move::build_move(
        Move::Flag::CASTLE_QUEENSIDE, ksq, ksq-2, CH::KING, 0
    ) : 0;
    search_index += qk&2 && (QS_MASK & (check_ray | bp->occ)) == 0;
}

// sort the 128 moves stored in the move array at index ply*128
// ply is some nonnegative value 0-29 inclusive
void MoveGen::sort_moves(int ply) {
    Move::move32 temp = 0;
    int start = ply<<7;
    int i = 1 + start;
    int end = ply+1<<7;
    while (i < end && search_moves_128x30[i]) {
        Move::move32 temp = search_moves_128x30[i];
        int j = i;
        while (j > start && search_moves_128x30[j-1] > temp) {
            search_moves_128x30[j] = search_moves_128x30[j-1];
            j--;
        }
        search_moves_128x30[j] = temp;
        i++;
    }
    return;
}

Board::board_type* MoveGen::make_move(Board::board_type* bp, Move::move32 mv) {
    Board::board_type* newb = Board::copy(bp);
    int flag  = mv       & 15;
    int start = mv >>  4 & 63;
    int end   = mv >> 10 & 63;
    int piece = mv >> 16 &  7;
    U64 sq0   = BB::sq_bb[start];
    U64 sq1   = BB::sq_bb[end];
    // moving piece:
    *newb->bb_piece[piece]                                          ^= sq0;
    *newb->bb_piece[flag&Move::Flag::PROMOTION ? (mv&7)+1 : piece]  ^= sq1;
    *newb->bb_color[newb->black_to_move]                            ^= sq0 | sq1;
    // captured piece:
    if (flag & Move::Flag::CAPTURE && flag != Move::Flag::EN_PASSANT) {
        *newb->bb_piece[(mv>>22&7)-1]         ^= sq1;
        *newb->bb_color[!newb->black_to_move] ^= sq1;
    }
    // castling kingside:
    *newb->bb_piece[CH::ROOK]            ^= flag == Move::Flag::CASTLE_KINGSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::f8] | BB::sq_bb[SQ::h8]) : (BB::sq_bb[SQ::f1] | BB::sq_bb[SQ::h1]) : 0ull;
    *newb->bb_color[newb->black_to_move] ^= flag == Move::Flag::CASTLE_KINGSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::f8] | BB::sq_bb[SQ::h8]) : (BB::sq_bb[SQ::f1] | BB::sq_bb[SQ::h1]) : 0ull;
    // castling queenside:
    *newb->bb_piece[CH::ROOK]            ^= flag == Move::Flag::CASTLE_QUEENSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::d8] | BB::sq_bb[SQ::a8]) : (BB::sq_bb[SQ::d1] | BB::sq_bb[SQ::a1]) : 0ull;
    *newb->bb_color[newb->black_to_move] ^= flag == Move::Flag::CASTLE_QUEENSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::d8] | BB::sq_bb[SQ::a8]) : (BB::sq_bb[SQ::d1] | BB::sq_bb[SQ::a1]) : 0ull;
    // en passant:
    *newb->bb_piece[CH::PAWN]             ^= flag == Move::Flag::EN_PASSANT ? BB::gen_shift(sq1, Dir::PAWN_DIR[!newb->black_to_move]) : 0ull;
    *newb->bb_color[!newb->black_to_move] ^= flag == Move::Flag::EN_PASSANT ? BB::gen_shift(sq1, Dir::PAWN_DIR[!newb->black_to_move]) : 0ull;
    // store ep file
    newb->ep_file = flag == Move::Flag::PAWN_DOUBLE_ADVANCE ? start & 7 : 8;
    // occupancy bitboard
    newb->occ = newb->black | newb->white;
    // halfmove counter
    newb->halfmoves = piece == CH::PAWN || (flag & Move::Flag::CAPTURE) ? 0 : newb->halfmoves + 1;
    // if black's move increment fullmoves
    newb->fullmoves += newb->black_to_move;
    // turn player
    newb->black_to_move = !newb->black_to_move;
    return newb;
}

// Move generator test methods:

// Root test method
void MoveGen::perft_root(Board::board_type* bp, int d) {
    const std::string PERFT_RESULTS[16] = {
        "1", "20", "400", "8902", "197281", "4865609", "119060324", "3195901860",
        "84998978956", "2439530234167", "69352859712417", "2097651003696806",
        "62854969236701747", "1981066775000396239", "61885021521585529237",
        "2015099950053364471960"
    };
    if (bp == nullptr || d < 0) return;
    U64 nodes = d == 0 ? 1 : 0;
    search_ply = 0;
    clear_array();
    gen_moves(bp);
    sort_moves(search_ply);
    search_ply++;
    for (int idx = 0; d && idx < 128 && search_moves_128x30[idx]; idx++) {
        Move::move32 mv = search_moves_128x30[idx];
        // if (d > 1) std::cout << Move::name(mv) << ":" << std::endl;
        Board::board_type* newb = make_move(bp, mv);
        U64 diff = perft(newb, d - 1);
        std::cout << Move::name(mv) << " " << diff << std::endl;
        nodes += diff;
    }
    search_ply--;
    std::cout << "perft " << std::setw(2) << std::to_string(d) << ": " << std::to_string(nodes) << std::endl;
    if (bp->occ != 0xffff00000000ffffull) return;
    std::cout << "expected: " << (d<16 ? PERFT_RESULTS[d] : "?????") << std::endl;
}

// Recursive test method
U64 MoveGen::perft(Board::board_type* bp, int d) {
    if (d == 0) return 1;

    gen_moves(bp);
    sort_moves(search_ply);
    search_ply++;
    U64 count = 0;
    for (int idx = 0; idx < 128 && search_moves_128x30[idx+(search_ply-1<<7)]; idx++) {
        Board::board_type* next = make_move(bp, MoveGen::search_moves_128x30[idx+(search_ply-1<<7)]);
        U64 diff = perft(next, d-1);
        count += diff;

        // if (d > 0) {
            // if (d < 2) std::cout << "  ";
            // std::cout << "  " << Move::name(mv) << " " << std::to_string(diff) << std::endl;
        // }
    }
    search_ply--;
    clear_array(128*search_ply);
    return count;
}
