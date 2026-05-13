#ifndef CHESS_ENGINE_BOARD_H
#define CHESS_ENGINE_BOARD_H
#include "Move.h"
#include "types.h"

#include <string>

class Position {
public:
  void print() const;

  void set(const std::string &fen);
  std::string fen() const;

  bool empty(Square square) const;
  inline Piece pieceOn(Square square) const;

  void makeMove(Move move);

  Bitboard getPieces() const;
  Bitboard getPieces(PieceType piece_type) const;
  Bitboard getPieces(Colour colour) const;
  Bitboard getPieces(PieceType piece_type, Colour colour) const;

  Colour getSide() const { return side_to_move; }
  CastlingRights getCastleRights() const { return castling_rights; }
  Square getEp() const { return ep_square; }

private:
  std::array<Piece, SQ_NB> board{};
  std::array<Bitboard, PIECE_TYPE_NB> by_type{};
  std::array<Bitboard, COLOUR_NB> by_colour{};

  Colour side_to_move{};
  CastlingRights castling_rights{};
  Square ep_square{};
  int halfmove_clock{};
  int fullmove_count{};
  Key hash{};

  void addPiece(Piece piece, Square sq);
  void removePiece(Square sq);
  void movePiece(Square from, Square to);
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

inline Bitboard Position::getPieces(const PieceType piece_type,
                                    const Colour colour) const {
  return by_type[piece_type] & by_colour[colour];
}
#endif // CHESS_ENGINE_BOARD_H
