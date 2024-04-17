// board.cpp
// struct to contain all information about a gamestate
// and the methods to build a new board from FEN or starting position

#include <iostream>

#include "board.h"
#include "chess.h"
#include "square.h"
#include "hash.h"


// build a board with bitboards for the starting position:
Board::board_type* Board::new_game() {
    board_type* newb    = new board_type;
    newb->white         = 0b1111111111111111ull;
    newb->black         = newb->white   << 8 * 6;
    newb->pawns         = 0b11111111ull << 8 * 6 | 0b11111111ull << 8;
    newb->knights       = 0b01000010ull << 8 * 7 | 0b01000010ull;
    newb->bishops       = 0b00100100ull << 8 * 7 | 0b00100100ull;
    newb->rooks         = 0b10000001ull << 8 * 7 | 0b10000001ull;
    newb->queens        = 0b00001000ull << 8 * 7 | 0b00001000ull;
    newb->kings         = 0b00010000ull << 8 * 7 | 0b00010000ull;
    newb->occ           = newb->white            | newb->black;
    newb->ep_file       = 8;
    newb->castle_qkQK   = 0b1111;
    newb->black_to_move = false;
    return newb;
}

// build a board that is a copy of the given board
Board::board_type* Board::copy(board_type* bp) {
    board_type* newb    = new board_type;
    newb->white         = bp->white;
    newb->black         = bp->black;
    newb->pawns         = bp->pawns;
    newb->knights       = bp->knights;
    newb->bishops       = bp->bishops;
    newb->rooks         = bp->rooks;
    newb->queens        = bp->queens;
    newb->kings         = bp->kings;
    newb->occ           = bp->occ;
    newb->ep_file       = bp->ep_file;
    newb->castle_qkQK   = bp->castle_qkQK;
    newb->black_to_move = bp->black_to_move;
    return newb;
}

void Board::print_bitboards(board_type* bp) {
    BB::print_binary_string(BB::build_binary_string(bp->occ),     "occ");
    BB::print_binary_string(BB::build_binary_string(bp->white),   "white");
    BB::print_binary_string(BB::build_binary_string(bp->black),   "black");
    BB::print_binary_string(BB::build_binary_string(bp->pawns),   "pawns");
    BB::print_binary_string(BB::build_binary_string(bp->knights), "knights");
    BB::print_binary_string(BB::build_binary_string(bp->bishops), "bishops");
    BB::print_binary_string(BB::build_binary_string(bp->rooks),   "rooks");
    BB::print_binary_string(BB::build_binary_string(bp->queens),  "queens");
    BB::print_binary_string(BB::build_binary_string(bp->kings),   "kings");
}

/**
 * decode the FEN string here:
 * rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 * Fields:
 * 0: Pieces
 * 1: Player to move
 * 2: Castle rights
 * 3: Ep target square
 * 4: Halfmove clock: number of moves since last capture/pawn advance
 * 5: Fullmove clock: starts at 1 and counts after black's moves
*/
Board::board_type* Board::build_fen(std::string fen) {
    board_type* newb = new board_type;
    int loc = 0;
    // Field 1: piece locations
    U64 sq = 1ull << 56;
    while (fen[loc] != ' ') {
        sq = sq >> (fen[loc] == '/') * 8;
        loc += fen[loc] == '/';
        if ('1' <= fen[loc] && fen[loc] <= '8') {
            sq = sq << fen[loc] - '0';
        } else {
            *newb->bb_piece[CH::char_to_piece(fen[loc])] |= sq;
            *newb->bb_color[CH::char_to_color(fen[loc])] |= sq;
            sq = sq << 1;
        }
        loc++;
    }
    newb->occ = newb->white | newb->black;
    loc++;
    // Field 2: active color
    newb->black_to_move = fen[loc] == 'b';
    loc += 2;
    // Field 3: castle availability
    if (fen[loc] != '-') {
        while (fen[loc] != ' ') {
            newb->castle_qkQK |= (fen[loc] == 'K') * 0b0001;
            newb->castle_qkQK |= (fen[loc] == 'Q') * 0b0010;
            newb->castle_qkQK |= (fen[loc] == 'k') * 0b0100;
            newb->castle_qkQK |= (fen[loc] == 'q') * 0b1000;
            loc++;
        }
    } else loc++;
    loc++;
    // Field 4: ep target square
    if (fen[loc] != '-') {
        newb->ep_file = SQ::square_from_string(fen.substr(loc, 2))&7;
        loc += 3;
    } else {
        newb->ep_file = 8;
        loc += 2;
    }
    // Field 5: halfmove clock
    newb->halfmoves = 0;
    while (fen[loc] != ' ') {
        newb->halfmoves *= 10;
        newb->halfmoves += fen[loc] - '0';
        loc++;
    }
    loc++;
    // Field 6: fullmove clock
    newb->fullmoves = 0;
    while (fen[loc] != ' ') {
        newb->halfmoves *= 10;
        newb->halfmoves += fen[loc] - '0';
        loc++;
    }
    return newb;
}

bool Board::square_occupied(board_type* bp, int sq) {
    return bp->occ & 1ull << sq;
}

// return the color (0 white, 1 black) at the square
// assumes the square is occupied
bool Board::color_at(board_type* bp, int sq) {
    return bp->black>>sq&1;
}

int Board::piece_at(board_type* bp, int sq) {
    U64 mask = 1ull << sq;
    return (bp->pawns   & mask) ? CH::PAWN   :
           (bp->knights & mask) ? CH::KNIGHT :
           (bp->bishops & mask) ? CH::BISHOP :
           (bp->rooks   & mask) ? CH::ROOK   :
           (bp->queens  & mask) ? CH::QUEEN  :
           (bp->kings   & mask) ? CH::KING   : -1;
}

int Board::piece_bb(board_type* bp, U64 bb) {
    return (bp->pawns   & bb) ? CH::PAWN   :
           (bp->knights & bb) ? CH::KNIGHT :
           (bp->bishops & bb) ? CH::BISHOP :
           (bp->rooks   & bb) ? CH::ROOK   :
           (bp->queens  & bb) ? CH::QUEEN  :
           (bp->kings   & bb) ? CH::KING   : -1;
}

void Board::print_board(board_type* bp, bool black_to_move) {
    for (int rank = 0; rank < 8; rank++) {
        std::cout << " ";
        for (int file = 0; file < 8; file++) {
            int sq = black_to_move ? 8*rank + file
                    : 8*(7-rank) + file;
            int piece = piece_at(bp, sq);
            if (piece == -1) {
                std::cout << ". ";
            } else {
                int color = color_at(bp, sq);
                std::cout << CH::piece_char[6*color+piece] << " ";
            }
        }
        std::cout << std::endl;
    }
}
