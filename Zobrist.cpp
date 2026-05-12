#include "Zobrist.h"

#include <random>

Zobrist::Zobrist() {
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
std::uint64_t Zobrist::compute(const Position &pos) const {
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