#include "Position.h"

#include "Attacks.h"
#include "Move.h"
#include "Zobrist.h"
#include "types.h"

#include <cstring>
#include <iostream>
#include <sstream>

void Position::makeMove(const Move move) {
  const auto enemy{opposite(side_to_move)};
  const Square from{move.from()};
  const Square to{move.to()};
  const Piece piece{board[from]};

  const CastlingRights old_cr = static_cast<CastlingRights>(castling_rights);
  const int old_ep_file{(ep_square == SQ_NONE) ? FILE_NB : ep_square % 8};
  const int ep_offset{(side_to_move == WHITE) ? SOUTH : NORTH};

  if (move.isCapture() || typeOf(piece) == PAWN) {
    halfmove_clock = 0;
  } else {
    halfmove_clock++;
  }

  if (move.isCapture() && !move.isEnPassant()) {
    if (typeOf(pieceOn(to)) == ROOK && ((1ULL << to) & rook_start)) {
      // remove the castling rights of enemy if rook is captured on start square
      const int file{to % 8};
      if (file == FILE_A) { // queenside
        castling_rights &= ~(side_to_move == WHITE ? BLACK_OOO : WHITE_OOO);
      } else { // kingside
        castling_rights &= ~(side_to_move == WHITE ? BLACK_OO : WHITE_OO);
      }
    }
    removePiece(to);
  }

  // double push & en passant
  if (move.isDoublePawn()) {
    ep_square = static_cast<Square>(to + ep_offset);
  } else {
    ep_square = SQ_NONE;
  }
  if (move.isEnPassant()) {
    removePiece(static_cast<Square>(to + ep_offset));
    ep_square = SQ_NONE;
  }
  if (old_ep_file < FILE_NB) {
    hash ^= Zobrist::en_passant[old_ep_file];
  }
  if (ep_square != SQ_NONE) {
    hash ^= Zobrist::en_passant[ep_square % 8];
  }

  // promotion, otherwise normally move
  if (move.isPromotion()) {
    Piece promo_piece{typeToPiece(side_to_move, move.promotionType())};
    removePiece(from);
    addPiece(promo_piece, to);
  } else {
    movePiece(from, to);
  }

  // castling
  if (move.isCastling()) {
    Square rook_from, rook_to;
    if (move.flags() == KING_CASTLE) {
      rook_from = SQ_H1;
      rook_to = SQ_F1;
    } else {
      rook_from = SQ_A1;
      rook_to = SQ_D1;
    }

    if (side_to_move == BLACK) {
      // move to 8th rank for black castling
      rook_from = rook_from + SQ_A8;
      rook_to = rook_to + SQ_A8;
    }
    assert(!pieceOn(rook_to));
    movePiece(rook_from, rook_to);
  }

  // update castling rights if king or rook moves
  if (typeOf(piece) == KING) {
    castling_rights &=
        ~(side_to_move == WHITE ? WHITE_CASTLING : BLACK_CASTLING);
  } else if (typeOf(piece) == ROOK) {
    if (from == SQ_A1) {
      castling_rights &= ~WHITE_OOO;
    } else if (from == SQ_H1) {
      castling_rights &= ~WHITE_OO;
    } else if (from == SQ_A8) {
      castling_rights &= ~BLACK_OOO;
    } else if (from == SQ_H8) {
      castling_rights &= ~BLACK_OO;
    }
  }

  // update hash with new castling rights
  hash ^= Zobrist::castling[old_cr] ^ Zobrist::castling[castling_rights];

  side_to_move = enemy;
  hash ^= Zobrist::side;
  if (side_to_move == WHITE) {
    fullmove_count++;
  }
}

bool Position::isSquareAttacked(const Square sq, const Colour attacker) const {
  const Bitboard occupancy = getPieces();
  const Bitboard pawns = getPieces(PAWN, attacker);
  const Bitboard knights = getPieces(KNIGHT, attacker);
  const Bitboard bishops = getPieces(BISHOP, attacker);
  const Bitboard rooks = getPieces(ROOK, attacker);
  const Bitboard queens = getPieces(QUEEN, attacker);
  const Bitboard kings = getPieces(KING, attacker);

  return (Attacks::pawn_attacks[attacker][sq] & pawns) ||
         (Attacks::knight_attacks[sq] & knights) ||
         (Attacks::bishop_attacks(sq, occupancy) & (bishops | queens)) ||
         (Attacks::rook_attacks(sq, occupancy) & (rooks | queens)) ||
         (Attacks::king_attacks[sq] & kings);
}

void Position::print() const {
  for (int rank = RANK_8; rank >= 0; --rank) {
    std::cout << rank + 1 << "  ";
    for (int file = FILE_A; file < FILE_NB; ++file) {
      const int sq = rank * 8 + file;
      const auto mask = static_cast<Bitboard>(1ULL << sq);

      char c = '.';
      for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
        const auto piece_type = static_cast<PieceType>(pt);
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
  std::cout << "Castling rights:   "
            << castlingToString(static_cast<CastlingRights>(castling_rights))
            << '\n';
  std::cout << "En passant square: "
            << (ep_square == SQ_NONE ? "-" : squareToString(ep_square)) << '\n';
  std::cout << "Halfmove clock:    " << halfmove_clock << '\n';
  std::cout << "Fullmove count:    " << fullmove_count << '\n';
  std::cout << "Hash:              " << hash << '\n';
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
    }

    if (rank > RANK_1) {
      ss << '/';
    }
  }

  ss << ' ' << (side_to_move == WHITE ? 'w' : 'b') << ' '
     << castlingToString(static_cast<CastlingRights>(castling_rights)) << ' '
     << (ep_square == SQ_NONE ? "-" : squareToString(ep_square)) << ' '
     << halfmove_clock << ' ' << fullmove_count;

  return ss.str();
}

bool Position::empty(const Square square) const {
  assert(0 <= square && square < SQ_NB && "Square out of bounds");
  return pieceOn(square) == NO_PIECE;
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
