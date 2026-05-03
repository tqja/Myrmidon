#include "Position.h"
#include "types.h"

#include <cstring>
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
  std::cout << "Castling rights:   " << castling_rights << '\n';
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
      by_colour[colour] |= 1ULL << sq;
      by_type[getPieceType(piece)] |= 1ULL << sq;
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
    int file = token[0] - 'a';
    int rank = token[1] - '1';
    ep_square = static_cast<Square>(rank * 8 + file);
  }

  ss >> halfmove_clock >> fullmove_count;
}
