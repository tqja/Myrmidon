#ifndef MYRMIDON_ATTACKS_H
#define MYRMIDON_ATTACKS_H
#include "types.h"
#include <immintrin.h>

namespace Attacks {
extern Bitboard pawn_attacks[COLOUR_NB][SQ_NB];
extern Bitboard knight_attacks[SQ_NB];
extern Bitboard king_attacks[SQ_NB];
extern Bitboard rook_mask[SQ_NB];
extern Bitboard bishop_mask[SQ_NB];
extern Bitboard rook_table[SQ_NB][4096];
extern Bitboard bishop_table[SQ_NB][512];

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
