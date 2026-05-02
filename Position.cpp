#include "Position.h"
#include "types.h"
#include <iostream>
#include <sstream>


void Position::print() const {
    for (int rank = RANK_8; rank >= 0; --rank) {
        std::cout << rank + 1 << "  ";
        for (int file = FILE_A; file < FILE_NB; ++file) {
            const int sq = rank * 8 + file;
            const auto mask = static_cast<Bitboard>(1ULL << sq);

            char c = '.';
            for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
                auto piece_type = static_cast<PieceType>(pt);
                if (getPieces(piece_type) & getPieces(WHITE) & mask) {
                    c = piece_chars[WHITE][piece_type];
                }
                else if (getPieces(piece_type) & getPieces(BLACK) & mask) {
                    c = piece_chars[BLACK][piece_type];
                }
            }
            std::cout << c << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "   a b c d e f g h\n";
}

void Position::set(const std::string &fen) {
    std::istringstream ss(fen);

    std::string token;

    // piece positions
    ss >> token;
    int sq = SQ_A8;
    for (const char piece : token) {
        if (piece == '/') {
            sq -= 16;
        } else if (std::isdigit(piece)) {
            sq += piece - '0';
        } else {
            const Colour colour = std::isupper(piece) ? WHITE : BLACK;
            by_colour[colour] |= 1ULL << sq;
            by_type[getPieceType(piece)] |= 1ULL << sq;
            sq++;
        }
    }
}
