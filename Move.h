#ifndef CHESS_ENGINE_MOVE_H
#define CHESS_ENGINE_MOVE_H

#include "types.h"
#include <cstdint>

enum MoveFlags : std::uint8_t {
  QUIET = 0,
  DOUBLE_PAWN = 1,
  KING_CASTLE = 2,
  QUEEN_CASTLE = 3,
  CAPTURE = 4,
  EP_CAPTURE = 5,
  KNIGHT_PROMO = 8,
  BISHOP_PROMO = 9,
  ROOK_PROMO = 10,
  QUEEN_PROMO = 11,
  KNIGHT_PROMO_CAPTURE = 12,
  BISHOP_PROMO_CAPTURE = 13,
  ROOK_PROMO_CAPTURE = 14,
  QUEEN_PROMO_CAPTURE = 15
};

class Move {
public:
  Move() = default;
  constexpr explicit Move(const std::uint16_t d) : data(d) {};
  constexpr Move(const Square from, const Square to,
                 const MoveFlags flags = QUIET)
      : data(from | (to << 6) | (flags << 12)) {};
  constexpr Square from() const { return static_cast<Square>(data & 63); }
  constexpr Square to() const { return static_cast<Square>((data >> 6) & 63); }
  constexpr MoveFlags flags() const {
    return static_cast<MoveFlags>((data >> 12) & 15);
  }

  constexpr bool isDoublePawn() const { return (flags() == DOUBLE_PAWN); }
  constexpr bool isCapture() const {
    const MoveFlags f{flags()};
    return f == CAPTURE || f == EP_CAPTURE ||
           (f >= KNIGHT_PROMO_CAPTURE && f <= QUEEN_PROMO_CAPTURE);
  }
  constexpr bool isPromotion() const {
    const MoveFlags f{flags()};
    return f >= KNIGHT_PROMO && f <= QUEEN_PROMO_CAPTURE;
  }
  constexpr bool isEnPassant() const { return flags() == EP_CAPTURE; }
  constexpr bool isCastling() const {
    const MoveFlags f{flags()};
    return f == KING_CASTLE || f == QUEEN_CASTLE;
  }
  constexpr PieceType promotionType() const {
    return static_cast<PieceType>(((data >> 12) & 3) + KNIGHT);
  }

  constexpr bool operator==(const Move &rhs) const { return data == rhs.data; }
  constexpr bool operator!=(const Move &rhs) const { return data != rhs.data; }

  std::uint16_t raw() const { return data; }

private:
  std::uint16_t data{};
};

#endif // CHESS_ENGINE_MOVE_H
