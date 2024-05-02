// board.cpp
// struct to contain all information about a gamestate
// and the methods to build a new board from FEN or starting position

#include <iostream>

#include "board.h"
#include "chess.h"
#include "square.h"
#include "hash.h"


// build a board with bitboards for the starting position:
Board::board_type::board_type() {
    this->white         = 0b1111111111111111ull;
    this->black         = this->white   << 8 * 6;
    this->pawns         = 0b11111111ull << 8 * 6 | 0b11111111ull << 8;
    this->knights       = 0b01000010ull << 8 * 7 | 0b01000010ull;
    this->bishops       = 0b00100100ull << 8 * 7 | 0b00100100ull;
    this->rooks         = 0b10000001ull << 8 * 7 | 0b10000001ull;
    this->queens        = 0b00001000ull << 8 * 7 | 0b00001000ull;
    this->kings         = 0b00010000ull << 8 * 7 | 0b00010000ull;
    this->occ           = this->white            | this->black;
    this->ep_file       = 8;
    this->castle_qkQK   = 0b1111;
    this->black_to_move = false;
}

// build a board that is a copy of the given board
Board::board_type::board_type(board_type* bp) {
    this->white         = bp->white;
    this->black         = bp->black;
    this->pawns         = bp->pawns;
    this->knights       = bp->knights;
    this->bishops       = bp->bishops;
    this->rooks         = bp->rooks;
    this->queens        = bp->queens;
    this->kings         = bp->kings;
    this->occ           = bp->occ;
    this->ep_file       = bp->ep_file;
    this->castle_qkQK   = bp->castle_qkQK;
    this->black_to_move = bp->black_to_move;
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
Board::board_type::board_type(std::string fen) {
    int loc = 0;
    // Field 1: piece locations
    U64 sq = 1ull << 56;
    while (fen[loc] != ' ') {
        sq = sq >> (fen[loc] == '/') * 8;
        loc += fen[loc] == '/';
        if ('1' <= fen[loc] && fen[loc] <= '8') {
            sq = sq << fen[loc] - '0';
        } else {
            *this->bb_piece[CH::char_to_piece(fen[loc])] |= sq;
            *this->bb_color[CH::char_to_color(fen[loc])] |= sq;
            sq = sq << 1;
        }
        loc++;
    }
    this->occ = this->white | this->black;
    loc++;
    // Field 2: active color
    this->black_to_move = fen[loc] == 'b';
    loc += 2;
    // Field 3: castle availability
    if (fen[loc] != '-') {
        while (fen[loc] != ' ') {
            this->castle_qkQK |= (fen[loc] == 'K') * 0b0001;
            this->castle_qkQK |= (fen[loc] == 'Q') * 0b0010;
            this->castle_qkQK |= (fen[loc] == 'k') * 0b0100;
            this->castle_qkQK |= (fen[loc] == 'q') * 0b1000;
            loc++;
        }
    } else loc++;
    loc++;
    // Field 4: ep target square
    if (fen[loc] != '-') {
        this->ep_file = SQ::square_from_string(fen.substr(loc, 2))&7;
        loc += 3;
    } else {
        this->ep_file = 8;
        loc += 2;
    }
    // Field 5: halfmove clock
    this->halfmoves = 0;
    while (fen[loc] != ' ') {
        this->halfmoves *= 10;
        this->halfmoves += fen[loc] - '0';
        loc++;
    }
    loc++;
    // Field 6: fullmove clock
    this->fullmoves = 0;
    while (fen[loc] != ' ') {
        this->halfmoves *= 10;
        this->halfmoves += fen[loc] - '0';
        loc++;
    }
}

bool Board::board_type::square_occupied(int sq) {
    return this->occ & 1ull << sq;
}

// return the color (0 white, 1 black) at the square
// assumes the square is occupied
bool Board::board_type::color_at(int sq) {
    return this->black>>sq&1;
}

int Board::board_type::piece_at(int sq) {
    U64 mask = 1ull << sq;
    return (this->pawns   & mask) ? CH::PAWN   :
           (this->knights & mask) ? CH::KNIGHT :
           (this->bishops & mask) ? CH::BISHOP :
           (this->rooks   & mask) ? CH::ROOK   :
           (this->queens  & mask) ? CH::QUEEN  :
           (this->kings   & mask) ? CH::KING   : -1;
}

int Board::board_type::piece_bb(U64 bb) {
    return (this->pawns   & bb) ? CH::PAWN   :
           (this->knights & bb) ? CH::KNIGHT :
           (this->bishops & bb) ? CH::BISHOP :
           (this->rooks   & bb) ? CH::ROOK   :
           (this->queens  & bb) ? CH::QUEEN  :
           (this->kings   & bb) ? CH::KING   : -1;
}

void Board::board_type::print_board(bool black_to_move) {
    for (int rank = 0; rank < 8; rank++) {
        std::cout << " ";
        for (int file = 0; file < 8; file++) {
            int sq = black_to_move ? 8*rank + file
                    : 8*(7-rank) + file;
            int piece = piece_at(sq);
            if (piece == -1) {
                std::cout << ". ";
            } else {
                int color = color_at(sq);
                std::cout << CH::piece_char[6*color+piece] << " ";
            }
        }
        std::cout << std::endl;
    }
}

void Board::board_type::print_bitboards() {
    BB::print_binary_string(BB::build_binary_string(this->occ),     "occ");
    BB::print_binary_string(BB::build_binary_string(this->white),   "white");
    BB::print_binary_string(BB::build_binary_string(this->black),   "black");
    BB::print_binary_string(BB::build_binary_string(this->pawns),   "pawns");
    BB::print_binary_string(BB::build_binary_string(this->knights), "knights");
    BB::print_binary_string(BB::build_binary_string(this->bishops), "bishops");
    BB::print_binary_string(BB::build_binary_string(this->rooks),   "rooks");
    BB::print_binary_string(BB::build_binary_string(this->queens),  "queens");
    BB::print_binary_string(BB::build_binary_string(this->kings),   "kings");
}
