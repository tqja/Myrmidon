#ifndef MYRMIDON_ATTACKS_H
#define MYRMIDON_ATTACKS_H
#include "types.h"
#include <array>
#include <immintrin.h>

namespace Attacks {
extern std::array<std::array<Bitboard, SQ_NB>, COLOUR_NB> pawn_attacks;
extern std::array<Bitboard, SQ_NB> knight_attacks;
extern std::array<Bitboard, SQ_NB> king_attacks;
extern std::array<Bitboard, SQ_NB> rook_mask;
extern std::array<Bitboard, SQ_NB> bishop_mask;
extern std::array<std::array<Bitboard, 4096>, SQ_NB> rook_table;
extern std::array<std::array<Bitboard, 512>, SQ_NB> bishop_table;

void init();

inline Bitboard rook_attacks(const Square sq, const Bitboard occ) {
  return rook_table[sq][_pext_u64(occ, rook_mask[sq])];
}

inline Bitboard bishop_attacks(const Square sq, const Bitboard occ) {
  return bishop_table[sq][_pext_u64(occ, bishop_mask[sq])];
}

inline Bitboard queen_attacks(const Square sq, const Bitboard occ) {
  return rook_attacks(sq, occ) | bishop_attacks(sq, occ);
}

} // namespace Attacks

#endif // MYRMIDON_ATTACKS_H
