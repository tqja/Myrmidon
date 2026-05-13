#include "Position.h"

#include "Zobrist.h"
#include "types.h"

#include <cstring>
#include <iostream>
#include <sstream>

void Position::makeMove(const Move move) {
  const Colour ally{side_to_move};
  const auto enemy{static_cast<Colour>(ally ^ 1)};
  const Square from{move.from()};
  const Square to{move.to()};
  const Piece piece{board[from]};

  if (move.isCapture() || typeOf(piece) == PAWN) {
    halfmove_clock = 0;
  } else {
    halfmove_clock++;
  }
  if (move.isCapture()) {
    removePiece(to);
  }

  movePiece(from, to);

  side_to_move = enemy;
  hash ^= Zobrist::side;
  if (side_to_move == WHITE) {
    fullmove_count++;
  }
}

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
        } else if (getPieces(piece_type) & getPieces(BLACK) & mask) {
          c = piece_chars[BLACK][piece_type];
        }
      }
      std::cout << c << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "   a b c d e f g h\n\n";
  std::cout << "Side to move:      "
            << (side_to_move == WHITE ? "White" : "Black") << "\n";
  std::cout << "Castling rights:   " << castlingToString(castling_rights)
            << '\n';
  std::cout << "En passant square: "
            << (ep_square == SQ_NONE ? "-" : squareToString(ep_square)) << '\n';
  std::cout << "Halfmove clock:    " << halfmove_clock << '\n';
  std::cout << "Fullmove count:    " << fullmove_count << '\n';
}

void Position::set(const std::string &fen) {
  std::istringstream ss(fen);
  std::string token;

  std::memset(reinterpret_cast<char *>(this), 0, sizeof(Position));

  // piece positions
  ss >> token;
  int sq = SQ_A8;
  for (const unsigned char piece : token) {
    if (piece == '/') {
      sq -= 16;
    } else if (std::isdigit(piece)) {
      sq += piece - '0';
    } else {
      const Colour colour = std::isupper(piece) ? WHITE : BLACK;
      board[sq] = charToPiece(piece);
      by_colour[colour] |= 1ULL << sq;
      by_type[charToPieceType(piece)] |= 1ULL << sq;
      sq++;
    }
  }

  // side to move
  ss >> token;
  if (token != "w" && token != "b") {
    std::cerr << token << " is not a valid side.";
    assert(false);
  }
  side_to_move = token == "w" ? WHITE : BLACK;

  // castling rights
  ss >> token;
  for (const char c : token) {
    switch (c) {
    case 'K':
      castling_rights |= WHITE_OO;
      break;
    case 'k':
      castling_rights |= BLACK_OO;
      break;
    case 'Q':
      castling_rights |= WHITE_OOO;
      break;
    case 'q':
      castling_rights |= BLACK_OOO;
      break;
    default:
      break;
    }
  }

  // en passant square
  ss >> token;
  if (token == "-") {
    ep_square = SQ_NONE;
  } else {
    const int file = token[0] - 'a';
    const int rank = token[1] - '1';
    ep_square = static_cast<Square>(rank * 8 + file);
  }

  ss >> halfmove_clock >> fullmove_count;
  hash = Zobrist::compute(*this);
}
std::string Position::fen() const {
  std::ostringstream ss;

  for (int r = RANK_8; r >= RANK_1; --r) {
    int empty_count{};
    const auto rank{static_cast<Rank>(r)};
    for (int f = FILE_A; f <= FILE_H; ++f) {
      const auto file{static_cast<File>(f)};
      const Square sq = makeSquare(file, rank);
      if (empty(sq)) {
        empty_count++;
        continue;
      }

      if (empty_count > 0) {
        ss << empty_count;
        empty_count = 0;
      }

      for (PieceType pt = PAWN; pt <= KING; ++pt) {
        if (getPieces(pt) & getPieces(WHITE) & (1ULL << sq)) {
          ss << piece_chars[WHITE][pt];
          break;
        } else if (getPieces(pt) & getPieces(BLACK) & (1ULL << sq)) {
          ss << piece_chars[BLACK][pt];
          break;
        }
      }
    }

    if (empty_count > 0) {
      ss << empty_count;
      empty_count = 0;
    }

    if (rank > RANK_1) {
      ss << '/';
    }
  }

  ss << ' ' << (side_to_move == WHITE ? 'w' : 'b') << ' '
     << castlingToString(castling_rights) << ' '
     << (ep_square == SQ_NONE ? "-" : squareToString(ep_square)) << ' '
     << halfmove_clock << ' ' << fullmove_count;

  return ss.str();
}

bool Position::empty(const Square square) const {
  assert(0 <= square && square < SQ_NB && "Square out of bounds");
  return pieceOn(square) == NO_PIECE;
}
Piece Position::pieceOn(const Square square) const {
  assert(0 <= square && square < SQ_NB && "Square out of bounds");
  return board[square];
}

void Position::addPiece(const Piece piece, const Square sq) {
  assert(piece != NO_PIECE);
  assert(board[sq] == NO_PIECE);

  const Colour c{colourOf(piece)};
  const PieceType p{typeOf(piece)};

  board[sq] = piece;
  by_colour[c] |= 1ULL << sq;
  by_type[p] |= 1ULL << sq;
  hash ^= Zobrist::pieceKey(piece, sq);
}

void Position::removePiece(const Square sq) {
  const Piece piece{board[sq]};
  assert(piece != NO_PIECE);

  const Colour c{colourOf(piece)};
  const PieceType p{typeOf(piece)};

  board[sq] = NO_PIECE;
  by_colour[c] &= ~(1ULL << sq);
  by_type[p] &= ~(1ULL << sq);
  hash ^= Zobrist::pieceKey(piece, sq);
}

void Position::movePiece(const Square from, const Square to) {
  const Piece piece{board[from]};
  assert(piece != NO_PIECE);
  removePiece(from);
  addPiece(piece, to);
}
