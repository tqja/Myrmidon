#ifndef MYRMIDON_ATTACKS_H
#define MYRMIDON_ATTACKS_H
#include "types.h"

namespace Attacks {
extern Bitboard pawn_attacks[COLOUR_NB][SQ_NB];
extern Bitboard knight_attacks[SQ_NB];
extern Bitboard king_attacks[SQ_NB];

void init();
} // namespace Attacks

#endif // MYRMIDON_ATTACKS_H
