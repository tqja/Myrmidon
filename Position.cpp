#include "Position.h"

#include "Attacks.h"
#include "Move.h"
#include "Zobrist.h"
#include "types.h"

#include <cstring>
#include <iostream>
#include <sstream>

void Position::makeMove(const Move move, StateInfo &new_st) {
  // copy board state to history
  std::memcpy(&new_st, st, offsetof(StateInfo, hash));
  new_st.previous = st;
  st = &new_st;

  const auto enemy{opposite(side_to_move)};
  const Square from{move.from()};
  const Square to{move.to()};
  const Piece piece{board[from]};

  const int old_ep_file{(st->ep_square == SQ_NONE) ? FILE_NB
                                                   : makeFile(st->ep_square)};
  const int ep_offset{(side_to_move == WHITE) ? SOUTH : NORTH};

  setHalfmoveClock(move, piece);

  if (move.isCapture() && !move.isEnPassant()) {
    removeEnemyCastleRightsOnCapture(to);
    st->captured = pieceOn(to);
    removePiece(to);
  }

  if (move.isDoublePawn()) {
    st->ep_square = static_cast<Square>(to + ep_offset);
  } else {
    st->ep_square = SQ_NONE;
  }

  if (move.isEnPassant()) {
    st->captured = pieceOn(static_cast<Square>(to + ep_offset));
    removePiece(static_cast<Square>(to + ep_offset));
    st->ep_square = SQ_NONE;
  }

  if (old_ep_file < FILE_NB) {
    st->hash ^= Zobrist::en_passant[old_ep_file];
  }
  if (st->ep_square != SQ_NONE) {
    st->hash ^= Zobrist::en_passant[makeFile(st->ep_square)];
  }

  if (move.isPromotion()) {
    Piece promo_piece{typeToPiece(side_to_move, move.promotionType())};
    removePiece(from);
    addPiece(promo_piece, to);
  } else {
    movePiece(from, to);
  }

  if (move.isCastling()) {
    auto [rook_from, rook_to]{getRookCastlingSquares(move)};
    movePiece(rook_from, rook_to);
  };

  updateCastlingRights(piece, from);

  side_to_move = enemy;
  st->hash ^= Zobrist::side;
  game_ply++;
}

void Position::unmakeMove(const Move move) {
  side_to_move = opposite(side_to_move);
  const Square from{move.from()};
  const Square to{move.to()};

  if (move.isPromotion()) {
    const Piece ally_pawn{typeToPiece(side_to_move, PAWN)};
    removePiece(to);
    addPiece(ally_pawn, to);
  }

  if (move.isCastling()) {
    movePiece(to, from);
    const File king_file{makeFile(to)};
    const bool king_side{king_file == FILE_G};
    Square rook_from{};
    Square rook_to{};
    if (king_side) {
      rook_from = static_cast<Square>(to + 1);
      rook_to = static_cast<Square>(to - 1);
    } else {
      rook_from = static_cast<Square>(to - 2);
      rook_to = static_cast<Square>(to + 1);
    }
    movePiece(rook_to, rook_from);
  } else {
    movePiece(to, from);

    if (st->captured) {
      Square capture_sq{to};
      const int ep_offset{(side_to_move == WHITE) ? SOUTH : NORTH};

      if (move.isEnPassant()) {
        capture_sq += static_cast<Square>(ep_offset);
      }

      addPiece(st->captured, capture_sq);
    }
  }

  st = st->previous;
  --game_ply;
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

bool Position::isMoveLegal(const Move move) {
  const Colour ally{side_to_move};
  StateInfo st{};
  makeMove(move, st);

  Bitboard king{by_type[KING] & by_colour[ally]};
  const Square king_sq{popLsb(king)};

  const bool legal{!isSquareAttacked(king_sq, opposite(ally))};
  unmakeMove(move);
  return legal;
}

void Position::print() const {
  for (int rank = RANK_8; rank >= 0; --rank) {
    std::cout << rank + 1 << "  ";
    for (int file = FILE_A; file < FILE_NB; ++file) {
      const Square sq{makeSquare(file, rank)};
      const auto mask = static_cast<Bitboard>(1ULL << sq);

      char c = '.';
      for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
        if (getPieces(pt) & getPieces(WHITE) & mask) {
          c = piece_chars[WHITE][pt];
        } else if (getPieces(pt) & getPieces(BLACK) & mask) {
          c = piece_chars[BLACK][pt];
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
            << castlingToString(
                   static_cast<CastlingRights>(st->castling_rights))
            << '\n';
  std::cout << "En passant square: "
            << (st->ep_square == SQ_NONE ? "-" : squareToString(st->ep_square))
            << '\n';
  std::cout << "Halfmove clock:    " << st->halfmove_clock << '\n';
  std::cout << "Ply:               " << game_ply << '\n';
  std::cout << "Hash: " << st->hash << "\n\n";
}

void Position::set(const std::string &fen, StateInfo *si) {
  std::istringstream ss(fen);
  std::string token;

  std::memset(reinterpret_cast<char *>(this), 0, sizeof(Position));
  std::memset(si, 0, sizeof(StateInfo));
  st = si;

  // piece positions
  ss >> token;
  int sq{SQ_A8};
  for (const unsigned char piece : token) {
    if (piece == '/') {
      sq -= 16;
    } else if (std::isdigit(piece)) {
      sq += piece - '0';
    } else {
      const Colour colour{std::isupper(piece) ? WHITE : BLACK};
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
      st->castling_rights |= WHITE_OO;
      break;
    case 'k':
      st->castling_rights |= BLACK_OO;
      break;
    case 'Q':
      st->castling_rights |= WHITE_OOO;
      break;
    case 'q':
      st->castling_rights |= BLACK_OOO;
      break;
    default:
      break;
    }
  }

  // en passant square
  ss >> token;
  if (token == "-") {
    st->ep_square = SQ_NONE;
  } else {
    const int file = token[0] - 'a';
    const int rank = token[1] - '1';
    st->ep_square = makeSquare(file, rank);
  }

  ss >> st->halfmove_clock >> game_ply;
  game_ply = std::max(2 * (game_ply - 1), 0) + side_to_move;
  st->hash = Zobrist::compute(*this);
}
std::string Position::fen() const {
  std::ostringstream ss;

  for (int r = RANK_8; r >= RANK_1; --r) {
    int empty_count{};
    const auto rank{static_cast<Rank>(r)};
    for (int f = FILE_A; f <= FILE_H; ++f) {
      const auto file{static_cast<File>(f)};
      const Square sq{makeSquare(file, rank)};
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
     << castlingToString(static_cast<CastlingRights>(st->castling_rights))
     << ' ' << (st->ep_square == SQ_NONE ? "-" : squareToString(st->ep_square))
     << ' ' << st->halfmove_clock << ' ' << (game_ply / 2) + 1;

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
  st->hash ^= Zobrist::pieceKey(piece, sq);
}

void Position::removePiece(const Square sq) {
  const Piece piece{board[sq]};
  assert(piece != NO_PIECE);

  const Colour c{colourOf(piece)};
  const PieceType p{typeOf(piece)};

  board[sq] = NO_PIECE;
  by_colour[c] &= ~(1ULL << sq);
  by_type[p] &= ~(1ULL << sq);
  st->hash ^= Zobrist::pieceKey(piece, sq);
}

void Position::movePiece(const Square from, const Square to) {
  const Piece piece{board[from]};
  if (piece == NO_PIECE) {
    print();
  } 
  assert(piece != NO_PIECE);
  removePiece(from);
  addPiece(piece, to);
}

void Position::setHalfmoveClock(const Move move, const Piece piece) {
  if (move.isCapture() || typeOf(piece) == PAWN) {
    st->halfmove_clock = 0;
  } else {
    st->halfmove_clock++;
  }
}

void Position::removeEnemyCastleRightsOnCapture(const Square rook_sq) {
  const bool rook_on_start_square{
      static_cast<bool>((1ULL << rook_sq) & rook_start)};

  if (typeOf(pieceOn(rook_sq)) == ROOK && rook_on_start_square) {
    const int file{makeFile(rook_sq)};
    if (file == FILE_A) { // queenside
      st->castling_rights &= ~(side_to_move == WHITE ? BLACK_OOO : WHITE_OOO);
    } else { // kingside
      st->castling_rights &= ~(side_to_move == WHITE ? BLACK_OO : WHITE_OO);
    }
  }
}

std::pair<Square, Square> Position::getRookCastlingSquares(const Move move) {
  Square rook_from{};
  Square rook_to{};
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
  return {rook_from, rook_to};
}

void Position::updateCastlingRights(const Piece piece, const Square rook_sq) {
  st->hash ^= Zobrist::castling[st->castling_rights];

  if (typeOf(piece) == KING) {
    st->castling_rights &=
        ~(side_to_move == WHITE ? WHITE_CASTLING : BLACK_CASTLING);
  } else if (typeOf(piece) == ROOK) {
    if (rook_sq == SQ_A1) {
      st->castling_rights &= ~WHITE_OOO;
    } else if (rook_sq == SQ_H1) {
      st->castling_rights &= ~WHITE_OO;
    } else if (rook_sq == SQ_A8) {
      st->castling_rights &= ~BLACK_OOO;
    } else if (rook_sq == SQ_H8) {
      st->castling_rights &= ~BLACK_OO;
    }
  }

  st->hash ^= Zobrist::castling[st->castling_rights];
}
