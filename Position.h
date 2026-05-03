#ifndef CHESS_ENGINE_BOARD_H
#define CHESS_ENGINE_BOARD_H
#include "types.h"

#include <string>

class Position {
public:
    void print() const;

    void set(const std::string& fen);

    [[nodiscard]] Bitboard getPieces() const;
    [[nodiscard]] Bitboard getPieces(PieceType piece_type) const;
    [[nodiscard]] Bitboard getPieces(Colour colour) const;
    [[nodiscard]] Bitboard getPieces(PieceType piece_type, Colour colour) const;

private:
    Bitboard by_type[6]{};
    Bitboard by_colour[2]{};

    Colour side_to_move{};
    CastlingRights castling_rights{};
    Square ep_square{};
    int halfmove_clock{};
    int fullmove_count{};
};

inline Bitboard Position::getPieces() const {
    return by_colour[WHITE] | by_colour[BLACK];
}

inline Bitboard Position::getPieces(const PieceType piece_type) const {
    return by_type[piece_type];
}

inline Bitboard Position::getPieces(const Colour colour) const {
    return by_colour[colour];
}

inline Bitboard Position::getPieces(const PieceType piece_type, const Colour colour) const {
    return by_type[piece_type] & by_colour[colour];
}
#endif //CHESS_ENGINE_BOARD_H
