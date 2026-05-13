#include "Zobrist.h"
#include "Position.h"
#include <random>

namespace Zobrist {
Key pieces[PIECE_NB][SQ_NB]{};
Key castling[CASTLING_RIGHT_NB]{};
Key en_passant[FILE_NB]{};
Key side{};

void init() {
  std::mt19937_64 rng(6936582);
  std::uniform_int_distribution<Key> dist;

  for (auto &p : pieces) {
    for (auto &key : p) {
      key = dist(rng);
    }
  }
  for (auto &key : castling) {
    key = dist(rng);
  }
  for (auto &key : en_passant) {
    key = dist(rng);
  }
  side = dist(rng);
}

std::uint64_t compute(const Position &pos) {
  Key hash{0};

  for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
    const Piece p = pos.pieceOn(sq);
    if (p != NO_PIECE) {
      hash ^= pieces[p][sq];
    }
  }

  hash ^= castling[pos.getCastleRights()];

  if (pos.getEp() != SQ_NONE) {
    hash ^= en_passant[pos.getEp() % 8];
  }

  if (pos.getSide() == BLACK) {
    hash ^= side;
  }

  return hash;
}
Key pieceKey(const Piece piece, const Square sq) {
  return pieces[piece][sq];
}
} // namespace Zobrist
