
#include <iomanip>
#include "move_gen.h"
#include "compass.h"
#include "timer.h"


Move::move32 MoveGen::search_moves_128x30[128*30] {};
int MoveGen::search_ply {};
int MoveGen::search_index {};
bool MoveGen::check {};
bool MoveGen::double_check {};
U64 MoveGen::op_atk {};
U64 MoveGen::check_ray {};
U64 MoveGen::pins {};

void MoveGen::gen_moves(Board::board_type* bp) {
    search_index = search_ply << 7;
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
    check_ray = 0ull;

    if (!(king & op_atk)) // check only exists if king is attacked
        return;

    int ksq = BB::bit_scan_forward(king);
    U64 op  = *bp->bb_color[!bp->black_to_move];

    // knights
    check_ray = bp->knights & op & Compass::knight_attacks[ksq];
    check     = check_ray;

    // pawns
    U64 attackers  = BB::gen_shift(king & BB::NOT_H_FILE, Dir::PAWN_DIR[bp->black_to_move] + 1);
    attackers     |= BB::gen_shift(king & BB::NOT_A_FILE, Dir::PAWN_DIR[bp->black_to_move] - 1);
    attackers     &= op & bp->pawns;
    check_ray      = check_ray ? check_ray : attackers;
    check = check || attackers;

    // bishops, queens
    U64 op_bq     = op & (bp->queens | bp->bishops);
    attackers     = BB::NoEa_attacks(king, ~bp->occ);
    attackers    |= BB::NoWe_attacks(king, ~bp->occ);
    attackers    |= BB::SoEa_attacks(king, ~bp->occ);
    attackers    |= BB::SoWe_attacks(king, ~bp->occ);
    attackers    &= op_bq;
    double_check  = check && attackers;
    check         = check || attackers;
    // if this is the first check, record the attacking ray
    if (attackers && !check_ray) {
        attackers = BB::NoEa_attacks(king, ~bp->occ);
        check_ray = attackers & op_bq ? attackers : check_ray;
        attackers = BB::NoWe_attacks(king, ~bp->occ);
        check_ray = attackers & op_bq ? attackers : check_ray;
        attackers = BB::SoEa_attacks(king, ~bp->occ);
        check_ray = attackers & op_bq ? attackers : check_ray;
        attackers = BB::SoWe_attacks(king, ~bp->occ);
        check_ray = attackers & op_bq ? attackers : check_ray;
    }

    // rooks, queens
    U64 op_rq     = op & (bp->queens | bp->rooks);
    attackers     = BB::nort_attacks(king, ~bp->occ);
    attackers    |= BB::sout_attacks(king, ~bp->occ);
    attackers    |= BB::east_attacks(king, ~bp->occ);
    attackers    |= BB::west_attacks(king, ~bp->occ);
    attackers    &= op_rq;
    double_check  = double_check || check && attackers || BB::count_bits(attackers) > 1;
    check         = check || attackers;
    // if this is the first check, record the attacking ray
    if (attackers && !check_ray) {
        attackers = BB::nort_attacks(king, ~bp->occ);
        check_ray = attackers & op_rq ? attackers : check_ray;
        attackers = BB::sout_attacks(king, ~bp->occ);
        check_ray = attackers & op_rq ? attackers : check_ray;
        attackers = BB::east_attacks(king, ~bp->occ);
        check_ray = attackers & op_rq ? attackers : check_ray;
        attackers = BB::west_attacks(king, ~bp->occ);
        check_ray = attackers & op_rq ? attackers : check_ray;
    }
}

// pawn moves, captures, promotions, and en passant
void MoveGen::gen_pawn_moves(Board::board_type* bp) {
    U64 pawns = bp->pawns & *bp->bb_color[bp->black_to_move];
    U64 king  = bp->kings & *bp->bb_color[bp->black_to_move];

    // pawn single and double advances
    U64 moves = BB::gen_shift(pawns, Dir::PAWN_DIR[bp->black_to_move]) & ~bp->occ;
    U64 doubles = BB::gen_shift(moves & (bp->black_to_move ? BB::ROW_6 : BB::ROW_3), Dir::PAWN_DIR[bp->black_to_move]) & ~bp->occ;
    U64 promotions = moves & (BB::ROW_1 | BB::ROW_8);
    moves ^= promotions;
    // if in check, only allow moves that escape check
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
        search_moves_128x30[search_index]   = Move::build_move(Move::Flag::PROMOTE_QUEEN,  start, endsq, CH::PAWN, 0);
        search_moves_128x30[search_index+1] = Move::build_move(Move::Flag::PROMOTE_ROOK,   start, endsq, CH::PAWN, 0);
        search_moves_128x30[search_index+2] = Move::build_move(Move::Flag::PROMOTE_BISHOP, start, endsq, CH::PAWN, 0);
        search_moves_128x30[search_index+3] = Move::build_move(Move::Flag::PROMOTE_KNIGHT, start, endsq, CH::PAWN, 0);
        search_index += 4;
        promotions &= promotions - 1; // clear the LS1B
    }

    // is there an ep capture available?
    U64 ep_bb = bp->ep_file < 8 ? (BB::A_FILE << bp->ep_file & (bp->black_to_move ? BB::ROW_3 : BB::ROW_6)) : 0ull;
    // is the ep pawn delivering check?
    U64 ep_ray = (BB::gen_shift(check_ray, Dir::PAWN_DIR[bp->black_to_move]) == ep_bb) ? ep_bb : 0ull;

    // pawn captures east
    moves = BB::gen_shift(pawns & BB::NOT_H_FILE, Dir::DIRS[4+2*bp->black_to_move]) & (*bp->bb_color[!bp->black_to_move] | ep_bb);
    promotions = moves & (BB::ROW_1 | BB::ROW_8);
    moves ^= promotions;
    // if in check, only allow moves that escape check
    moves      &= check ? check_ray | ep_ray : ~0ull;
    promotions &= check ? check_ray          : ~0ull;
    while (moves) {
        int endsq = BB::bit_scan_forward(moves);
        int start = endsq - Dir::DIRS[4+2*bp->black_to_move];
        uint8_t flag = ep_bb & moves & -moves ? Move::Flag::EN_PASSANT : Move::Flag::CAPTURE;
        if (pins & BB::sq_bb[start]) { // is the moving pawn pinned?
            if (bp->black_to_move) {
                if (!(king & BB::NoWe_attacks(BB::sq_bb[start], ~bp->occ))) {
                    // if the capture is NOT along the pin ray (ie. capturing the pinning piece)
                    // then it is illegal
                    moves &= moves - 1; // clear the LS1B
                    continue;
                }
            } else {
                if (!(king & BB::SoWe_attacks(BB::sq_bb[start], ~bp->occ))) {
                    // if the capture is NOT along the pin ray (ie. capturing the pinning piece)
                    // then it is illegal
                    moves &= moves - 1; // clear the LS1B
                    continue;
                }
            }
        } else if (flag == Move::Flag::EN_PASSANT) {
            // TODO: this fixes the false "pin" of a pawn and its ep target to the king revealing an op's attacker
            // TODO: is there a better solution?
            U64 rooks = *bp->bb_color[!bp->black_to_move] & (bp->rooks | bp->queens);
            U64 empty = ~(bp->occ ^ (BB::sq_bb[start] | BB::gen_shift(ep_bb, Dir::PAWN_DIR[!bp->black_to_move])));
            if (rooks & (BB::east_attacks(king, empty) | BB::west_attacks(king, empty))) {
                moves &= moves - 1;
                continue;
            }
        }
        int target = flag == Move::Flag::EN_PASSANT ? CH::PAWN : bp->piece_at(endsq);

        search_moves_128x30[search_index] = Move::build_move(flag, start, endsq, CH::PAWN, target);
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    while (promotions) {
        int endsq = BB::bit_scan_forward(promotions);
        int start = endsq - Dir::DIRS[4+2*bp->black_to_move];
        int target = bp->piece_at(endsq);
        search_moves_128x30[search_index]   = Move::build_move(Move::Flag::CAPTURE_PROMOTE_QUEEN,  start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+1] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_ROOK,   start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+2] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_BISHOP, start, endsq, CH::PAWN, target);
        search_moves_128x30[search_index+3] = Move::build_move(Move::Flag::CAPTURE_PROMOTE_KNIGHT, start, endsq, CH::PAWN, target);
        search_index += 4;
        promotions &= promotions - 1; // clear the LS1B
    }

    // pawn captures west
    moves = BB::gen_shift(pawns & BB::NOT_A_FILE, Dir::DIRS[5+2*bp->black_to_move]) & (*bp->bb_color[!bp->black_to_move] | ep_bb);
    promotions = moves & (BB::ROW_1 | BB::ROW_8);
    moves ^= promotions;
    // if in check, only allow moves that escape check
    moves      &= check ? check_ray | ep_ray : ~0ull;
    promotions &= check ? check_ray          : ~0ull;
    while (moves) {
        int endsq = BB::bit_scan_forward(moves);
        int start = endsq - Dir::DIRS[5+2*bp->black_to_move];
        uint8_t flag = ep_bb & moves & -moves ? Move::Flag::EN_PASSANT : Move::Flag::CAPTURE;
        if (pins & BB::sq_bb[start]) { // is the moving pawn pinned?
            if (bp->black_to_move) {
                if (!(king & BB::NoEa_attacks(BB::sq_bb[start], ~bp->occ))) {
                    moves &= moves - 1; // clear the LS1B
                    continue;
                }
            } else {
                if (!(king & BB::SoEa_attacks(BB::sq_bb[start], ~bp->occ))) {
                    moves &= moves - 1; // clear the LS1B
                    continue;
                }
            }
        } else if (flag == Move::Flag::EN_PASSANT) {
            // TODO: this fixes the false "pin" of a pawn and its ep target to the king revealing an op's attacker
            // TODO: is there a better solution?
            U64 rooks = *bp->bb_color[!bp->black_to_move] & (bp->rooks | bp->queens);
            U64 empty = ~(bp->occ ^ (BB::sq_bb[start] | BB::gen_shift(ep_bb, Dir::PAWN_DIR[!bp->black_to_move])));
            if (rooks & (BB::east_attacks(king, empty) | BB::west_attacks(king, empty))) {
                moves &= moves - 1;
                continue;
            }
        }
        int target = flag == Move::Flag::EN_PASSANT ? CH::PAWN : bp->piece_at(endsq);
        search_moves_128x30[search_index] = Move::build_move(flag, start, endsq, CH::PAWN, target);
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    while (promotions) {
        int endsq = BB::bit_scan_forward(promotions);
        int start = endsq - Dir::DIRS[5+2*bp->black_to_move];
        int target = bp->piece_at(endsq);
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
                flag, start, end, CH::KNIGHT, flag ? bp->piece_bb(moves & -moves) : 0
            );
            search_index++;
            moves &= moves - 1; // clear the LS1B
        }
        knights &= knights - 1; // clear the LS1B
    }
}

void MoveGen::gen_bishop_moves(Board::board_type* bp) {
    U64 b = (bp->bishops | bp->queens) & *bp->bb_color[bp->black_to_move];
    U64 king = bp->kings & *bp->bb_color[bp->black_to_move];
    while (b) {
        U64 pin_ray = ~0ull;
        if (pins & b&-b) { // is the moving piece pinned?
            pin_ray = 0ull;
            if (BB::NoEa_attacks(b&-b, ~bp->occ) & king) {
                pin_ray = BB::SoWe_attacks(king, ~(bp->occ ^ b&-b));
            } else if (BB::NoWe_attacks(b&-b, ~bp->occ) & king) {
                pin_ray = BB::SoEa_attacks(king, ~(bp->occ ^ b&-b));
            } else if (BB::SoEa_attacks(b&-b, ~bp->occ) & king) {
                pin_ray = BB::NoWe_attacks(king, ~(bp->occ ^ b&-b));
            } else if (BB::SoWe_attacks(b&-b, ~bp->occ) & king) {
                pin_ray = BB::NoEa_attacks(king, ~(bp->occ ^ b&-b));
            }
        }
        int start = BB::bit_scan_forward(b);
        U64 moves = BB::NoEa_attacks(b&-b, ~bp->occ) | BB::NoWe_attacks(b&-b, ~bp->occ)
                  | BB::SoEa_attacks(b&-b, ~bp->occ) | BB::SoWe_attacks(b&-b, ~bp->occ);
        moves &= pin_ray;
        moves &= ~*bp->bb_color[bp->black_to_move];
        moves &= check ? check_ray : ~0ull;
        while (moves) {
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            int end = BB::bit_scan_forward(moves);
            search_moves_128x30[search_index] = Move::build_move(
                flag, start, end, bp->bishops & b & -b ? CH::BISHOP : CH::QUEEN, flag ? bp->piece_at(end) : 0
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
    U64 king = bp->kings & *bp->bb_color[bp->black_to_move];
    while (r) {
        U64 pin_ray = ~0ull;
        if (pins & r&-r) { // is the moving piece pinned?
            pin_ray = 0ull;
            if (BB::nort_attacks(r&-r, ~bp->occ) & king) {
                pin_ray = BB::sout_attacks(king, ~(bp->occ ^ r&-r));
            } else if (BB::sout_attacks(r&-r, ~bp->occ) & king) {
                pin_ray = BB::nort_attacks(king, ~(bp->occ ^ r&-r));
            } else if (BB::east_attacks(r&-r, ~bp->occ) & king) {
                pin_ray = BB::west_attacks(king, ~(bp->occ ^ r&-r));
            } else if (BB::west_attacks(r&-r, ~bp->occ) & king) {
                pin_ray = BB::east_attacks(king, ~(bp->occ ^ r&-r));
            }
        }
        int start = BB::bit_scan_forward(r);
        int piece = bp->rooks & r & -r ? CH::ROOK : CH::QUEEN;
        // East/West moves
        U64 moves = Compass::get_rank_attacks(bp->occ, start) & ~*bp->bb_color[bp->black_to_move];
        moves &= pin_ray;
        moves &= check ? check_ray : ~0ull;
        while (moves) {
            int end = BB::bit_scan_forward(moves);
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            int target = flag ? bp->piece_at(end) : 0;
            search_moves_128x30[search_index] = Move::build_move(flag, start, end, piece, target);
            search_index++;
            moves &= moves - 1; // clear the LS1B
        }
        // North/South moves
        int rstart = (((start >> 3) | (start << 3)) & 63) ^ 56;
        moves = Compass::get_rank_attacks(rocc, rstart) & ~rcol;
        moves = BB::rotate_counterclockwise(moves);
        moves &= pin_ray;
        moves &= check ? check_ray : ~0ull;
        while (moves) {
            int end = BB::bit_scan_forward(moves);
            uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
            int target = flag ? bp->piece_at(end) : 0;
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
    // normal moves/captures:
    while (moves) {
        uint8_t flag = moves & -moves & bp->occ ? Move::Flag::CAPTURE : 0;
        int end = BB::bit_scan_forward(moves & -moves);
        search_moves_128x30[search_index] = Move::build_move(
            flag, ksq, end, CH::KING, flag ? bp->piece_at(end) : 0
        );
        search_index++;
        moves &= moves - 1; // clear the LS1B
    }
    if (check) return; // no castling under check
    // castling:
    uint8_t qk = (bp->castle_qkQK >> 2 * bp->black_to_move) & 3;
    const U64 KS_CHECK = bp->black_to_move ? 96ull << 56 : 96; // 0b11 << 5 = 96
    const U64 QS_CHECK = bp->black_to_move ? 12ull << 56 : 12; // 0b11 << 2 = 12
    const U64 QS_OCC   = bp->black_to_move ? 2ull  << 56 : 2 ;
    search_moves_128x30[search_index] = qk&1 && !(KS_CHECK & (op_atk | bp->occ)) ? Move::build_move(
        Move::Flag::CASTLE_KINGSIDE, ksq, ksq+2, CH::KING, 0
    ) : 0;
    search_index += qk&1 && !(KS_CHECK & (op_atk | bp->occ ^ k));
    search_moves_128x30[search_index] = qk&2 && !(QS_CHECK & (op_atk | bp->occ)) && !(QS_OCC & bp->occ) ? Move::build_move(
        Move::Flag::CASTLE_QUEENSIDE, ksq, ksq-2, CH::KING , 0
    ) : 0;
    search_index += qk&2 && !(QS_CHECK & (op_atk | bp->occ ^ k)) && !(QS_OCC & bp->occ);
}

// sort the 128 moves stored in the move array at index ply*128
// ply is some nonnegative value 0-29 inclusive
void MoveGen::sort_moves(int ply) {
    Move::move32 temp = 0;
    int start = ply << 7;
    int i = 1 + start;
    const int end = ply+1 << 7;
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
    Board::board_type* newb = new Board::board_type(bp);
    int flag   = mv       & 15;
    int start  = mv >>  4 & 63;
    int end    = mv >> 10 & 63;
    int piece  = mv >> 16 &  7;
    int target = (mv >> 22 & 7) - 1;
    U64 st_bb  = BB::sq_bb[start];
    U64 end_bb = BB::sq_bb[end];

    // moving piece:
    *newb->bb_piece[piece]                                         ^= st_bb;
    *newb->bb_piece[flag&Move::Flag::PROMOTION ? (flag&3)+1:piece] ^= end_bb;
    *newb->bb_color[newb->black_to_move]                           ^= st_bb | end_bb;
    // captured piece:
    if (flag & Move::Flag::CAPTURE && flag != Move::Flag::EN_PASSANT) {
        *newb->bb_piece[target]               ^= end_bb;
        *newb->bb_color[!newb->black_to_move] ^= end_bb;
    }

    // castling kingside:
    *newb->bb_piece[CH::ROOK]            ^= flag == Move::Flag::CASTLE_KINGSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::f8] | BB::sq_bb[SQ::h8]) : (BB::sq_bb[SQ::f1] | BB::sq_bb[SQ::h1]) : 0ull;
    *newb->bb_color[newb->black_to_move] ^= flag == Move::Flag::CASTLE_KINGSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::f8] | BB::sq_bb[SQ::h8]) : (BB::sq_bb[SQ::f1] | BB::sq_bb[SQ::h1]) : 0ull;
    // castling queenside:
    *newb->bb_piece[CH::ROOK]            ^= flag == Move::Flag::CASTLE_QUEENSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::d8] | BB::sq_bb[SQ::a8]) : (BB::sq_bb[SQ::d1] | BB::sq_bb[SQ::a1]) : 0ull;
    *newb->bb_color[newb->black_to_move] ^= flag == Move::Flag::CASTLE_QUEENSIDE ? newb->black_to_move ? (BB::sq_bb[SQ::d8] | BB::sq_bb[SQ::a8]) : (BB::sq_bb[SQ::d1] | BB::sq_bb[SQ::a1]) : 0ull;
    // en passant:
    *newb->bb_piece[CH::PAWN]             ^= flag == Move::Flag::EN_PASSANT ? BB::gen_shift(end_bb, Dir::PAWN_DIR[!newb->black_to_move]) : 0ull;
    *newb->bb_color[!newb->black_to_move] ^= flag == Move::Flag::EN_PASSANT ? BB::gen_shift(end_bb, Dir::PAWN_DIR[!newb->black_to_move]) : 0ull;
    // store ep file
    newb->ep_file = flag == Move::Flag::PAWN_DOUBLE_ADVANCE ? start & 7 : 8;
    // occupancy bitboard
    newb->occ = newb->black | newb->white;
    // halfmove counter
    newb->halfmoves = piece == CH::PAWN || (flag & Move::Flag::CAPTURE) ? 0 : newb->halfmoves + 1;
    // if black's move increment fullmoves
    newb->fullmoves += newb->black_to_move;

    // update castle rights on king/rook moves
    // Kingside castle rights update
    newb->castle_qkQK = piece == CH::KING || (piece == CH::ROOK && start == (newb->black_to_move ? SQ::h8 : SQ::h1)) ?
                        newb->castle_qkQK & ~(newb->black_to_move ? 4 : 1) : newb->castle_qkQK;
    // Queenside castle rights update
    newb->castle_qkQK = piece == CH::KING || (piece == CH::ROOK && start == (newb->black_to_move ? SQ::a8 : SQ::a1)) ?
                        newb->castle_qkQK & ~(newb->black_to_move ? 8 : 2) : newb->castle_qkQK;

    // update castle rights on rook captures
    const SQ::square_type KS_RSQ[2] = { SQ::h1, SQ::h8 };
    const SQ::square_type QS_RSQ[2] = { SQ::a1, SQ::a8 };
    // Kingside rook captured
    newb->castle_qkQK = (flag & Move::Flag::CAPTURE && target == CH::ROOK && end == KS_RSQ[!newb->black_to_move]) ?
                        newb->castle_qkQK & ~(newb->black_to_move ? 1 : 4) : newb->castle_qkQK;
    // Queenside rook captured
    newb->castle_qkQK = (flag & Move::Flag::CAPTURE && target == CH::ROOK && end == QS_RSQ[!newb->black_to_move]) ?
                        newb->castle_qkQK & ~(newb->black_to_move ? 2 : 8) : newb->castle_qkQK;
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
    if (d < 0 || d > 30) return;

    Timer t;
    U64 nodes = d == 0 ? 1 : 0;
    search_ply = 0;
    clear_array();
    gen_moves(bp);
    // sort_moves(search_ply);
    search_ply++;
    for (int idx = 0; d && idx < 128 && search_moves_128x30[idx]; idx++) {
        Move::move32 mv = search_moves_128x30[idx];
        Board::board_type* newb = make_move(bp, mv);
        std::cout << Move::name(mv) << ": ";
        U64 diff = perft(newb, d - 1);
        nodes += diff;
        std::cout << diff << std::endl;
        delete newb;
    }
    search_ply--;
    std::cout << "perft " << std::setw(2) << std::to_string(d) << ": " << std::to_string(nodes) << std::endl;
    std::cout << "test completed in: " << t.elapsed() << " seconds." << std::endl;
    if (bp->occ != 0xffff00000000ffffull) return;
    std::cout << "expected: " << (d<16 ? PERFT_RESULTS[d] : "?????") << std::endl;
}

// Recursive test method
U64 MoveGen::perft(Board::board_type* bp, int d) {
    if (d == 0) return 1;

    gen_moves(bp);
    // sort_moves(search_ply);
    search_ply++;
    U64 count = 0;
    for (int idx = 0; idx < 128 && search_moves_128x30[idx+(search_ply-1<<7)]; idx++) {
        Board::board_type* next = make_move(bp, search_moves_128x30[idx+(search_ply-1<<7)]);
        U64 diff = perft(next, d-1);
        count += diff;
        delete next;
    }
    search_ply--;
    clear_array(search_ply << 7);
    return count;
}
