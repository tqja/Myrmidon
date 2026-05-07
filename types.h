#ifndef CHESS_ENGINE_TYPES_H
#define CHESS_ENGINE_TYPES_H
#include <cassert>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <ostream>

using Bitboard = std::uint64_t;

constexpr char piece_chars[2][7] = {
    {'P', 'N', 'B', 'R', 'Q', 'K', '.'}, // white
    {'p', 'n', 'b', 'r', 'q', 'k', '.'}, // black
};

enum Colour : std::uint8_t { WHITE, BLACK, COLOUR_NB };

// clang-format off
enum PieceType : std::uint8_t {
  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
  PIECE_TYPE_NB,
  NO_PIECE_TYPE = PIECE_TYPE_NB
};

// used for mailbox board
enum Piece : std::uint8_t {
  NO_PIECE, W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  B_PAWN = W_PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  PIECE_NB = 16
};

inline PieceType charToPieceType(const unsigned char c) {
  switch (std::tolower(c)) {
    case 'p': return PAWN;
    case 'n': return KNIGHT;
    case 'b': return BISHOP;
    case 'r': return ROOK;
    case 'q': return QUEEN;
    case 'k': return KING;
    default:
      std::cerr << c << " is not a valid piece type." << std::endl;
      assert(false);
    }
}

inline Piece charToPiece(const unsigned char c) {
  Piece piece;
  switch (std::tolower(c)) {
    case 'p': piece = W_PAWN; break;
    case 'n': piece = W_KNIGHT; break;
    case 'b': piece = W_BISHOP; break;
    case 'r': piece = W_ROOK; break;
    case 'q': piece = W_QUEEN; break;
    case 'k': piece = W_KING; break;
    default: return NO_PIECE;
  }

  return std::isupper(c) ? static_cast<Piece>(piece + 8) : piece;
}

enum Square : std::uint8_t {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE,

  SQ_ZERO = 0,
  SQ_NB = 64
};
// clang-format on

inline std::string squareToString(const Square sq) {
  char file = static_cast<char>('a' + (sq % 8));
  char rank = static_cast<char>('1' + (sq / 8));
  return {file, rank};
}

enum File : std::uint8_t {
  FILE_A,
  FILE_B,
  FILE_C,
  FILE_D,
  FILE_E,
  FILE_F,
  FILE_G,
  FILE_H,
  FILE_NB
};

enum Rank : std::uint8_t {
  RANK_1,
  RANK_2,
  RANK_3,
  RANK_4,
  RANK_5,
  RANK_6,
  RANK_7,
  RANK_8,
  RANK_NB
};

inline Square makeSquare(const File f, const Rank r) {
  return static_cast<Square>(f + r * 8);
}

#define ENABLE_INCR_OPERATORS_ON(T)                                            \
  constexpr T &operator++(T &d) {                                              \
    return d = static_cast<T>(static_cast<int>(d) + 1);                        \
  }                                                                            \
  constexpr T &operator--(T &d) {                                              \
    return d = static_cast<T>(static_cast<int>(d) - 1);                        \
  }

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON

enum Direction : std::int8_t {
  NORTH = 8,
  EAST = 1,
  SOUTH = -NORTH,
  WEST = -EAST,

  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  NORTH_WEST = NORTH + WEST
};

enum CastlingRights : std::uint8_t {
  NO_CASTLING,
  WHITE_OO,
  WHITE_OOO = WHITE_OO << 1,
  BLACK_OO = WHITE_OO << 2,
  BLACK_OOO = WHITE_OO << 3,

  KING_SIDE = WHITE_OO | BLACK_OO,
  QUEEN_SIDE = WHITE_OOO | BLACK_OOO,
  WHITE_CASTLING = WHITE_OO | WHITE_OOO,
  BLACK_CASTLING = BLACK_OO | BLACK_OOO,
  ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,

  CASTLING_RIGHT_NB = 16
};

inline std::string castlingToString(const CastlingRights cr) {
  if (cr == NO_CASTLING)
    return "-";
  std::string result{};
  if (cr & WHITE_OO) {
    result += 'K';
  }
  if (cr & WHITE_OOO) {
    result += 'Q';
  }
  if (cr & BLACK_OO) {
    result += 'k';
  }
  if (cr & BLACK_OOO) {
    result += 'q';
  }
  return result;
}

constexpr CastlingRights operator|(const CastlingRights a,
                                   const CastlingRights b) {
  return static_cast<CastlingRights>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr CastlingRights &operator|=(CastlingRights &a,
                                     const CastlingRights b) {
  return a = a | b;
}

#endif // CHESS_ENGINE_TYPES_H
