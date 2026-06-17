#ifndef MYRMIDON_POSITION_H
#define MYRMIDON_POSITION_H
#include "Move.h"
#include "types.h"

#include <array>
#include <string>

struct StateInfo {
  Square ep_square;
  CastlingRights castling_rights;
  int halfmove_clock;
  Key hash;

  Piece captured;
  StateInfo* previous;
};

struct History {
  StateInfo st;
  std::array<Bitboard, PIECE_TYPE_NB> by_piece;
  std::array<Bitboard, COLOUR_NB> by_colour;
  std::array<Bitboard, SQ_NB> board;
};

class Position {
public:
  std::array<Bitboard, COLOUR_NB> by_colour{};
  void print() const;

  void set(const std::string &fen, StateInfo *si);
  std::string fen() const;

  bool empty(Square square) const;

  void makeMove(Move move, StateInfo &new_st);
  void unmakeMove(Move move);
  bool isMoveLegal(Move move);

  Bitboard getPieces() const;
  Bitboard getPieces(int piece_type) const;
  Bitboard getPieces(Colour colour) const;
  Bitboard getPieces(int piece_type, Colour colour) const;

  Colour getSide() const { return side_to_move; }
  CastlingRights getCastleRights() const {
    return static_cast<CastlingRights>(st->castling_rights);
  }
  Square getEp() const { return st->ep_square; }

  bool isSquareAttacked(Square sq, Colour attacker) const;
  bool kingLeftInCheck() const;

  inline Piece pieceOn(const Square square) const {
    assert(0 <= square && square < SQ_NB && "Square out of bounds");
    return board[square];
  }

private:
  StateInfo *st{};
  std::array<History, MAX_PLY> history{};
  std::array<Bitboard, PIECE_TYPE_NB> by_type{};
  std::array<Piece, SQ_NB> board{};
  Colour side_to_move;
  int game_ply{};

  void addPiece(Piece piece, Square sq);
  void removePiece(Square sq);
  void movePiece(Square from, Square to);

  void setHalfmoveClock(Move move, const Piece piece);
  void removeEnemyCastleRightsOnCapture(Square to, Colour enemy);
  std::pair<Square, Square> getRookCastlingSquares(Move move);
  void updateCastlingRights(Piece piece, Square rook_sq);
};

inline Bitboard Position::getPieces() const {
  return by_colour[WHITE] | by_colour[BLACK];
}

inline Bitboard Position::getPieces(const int piece_type) const {
  return by_type[piece_type];
}

inline Bitboard Position::getPieces(const Colour colour) const {
  return by_colour[colour];
}

inline Bitboard Position::getPieces(const int piece_type,
                                    const Colour colour) const {
  return by_type[piece_type] & by_colour[colour];
}
#endif // MYRMIDON_POSITION_H
