#include "Attacks.h"

#include <cstdlib>

namespace Attacks {
Bitboard pawn_attacks[COLOUR_NB][SQ_NB]{};
Bitboard knight_attacks[SQ_NB]{};
Bitboard king_attacks[SQ_NB]{};

static Bitboard initWhitePawnAttacks(const Square sq, const Rank rank,
                                     const File file) {
  Bitboard pawn_bb{};
  if (RANK_1 < rank && rank < RANK_8) {
    if (file > FILE_A) {
      pawn_bb |= 1ULL << (sq + static_cast<int>(NORTH_WEST));
    }
    if (file < FILE_H) {
      pawn_bb |= 1ULL << (sq + static_cast<int>(NORTH_EAST));
    }
  }
  return pawn_bb;
}

static Bitboard initBlackPawnAttacks(const Square sq, const Rank rank,
                                     const File file) {
  Bitboard pawn_bb{};
  if (RANK_1 < rank && rank < RANK_8) {
    if (file > FILE_A) {
      pawn_bb |= 1ULL << (sq + static_cast<int>(SOUTH_WEST));
    }
    if (file < FILE_H) {
      pawn_bb |= 1ULL << (sq + static_cast<int>(SOUTH_EAST));
    }
  }
  return pawn_bb;
}

static Bitboard initKnightAttacks(const Square sq, const Rank rank,
                                  const File file) {
  Bitboard knight_bb{};
  constexpr int offsets[] = {15, 17, 10, -6, -15, -17, -10, 6};

  for (const int offset : offsets) {
    const int target = sq + offset;
    if (target < SQ_A1 || target >= SQ_NB) {
      continue;
    }

    const int target_rank{target / 8};
    const int target_file{target % 8};

    // check if the knight landed in bounds or not
    if (abs(target_rank - rank) <= 2 && abs(target_file - file) <= 2) {
      knight_bb |= 1ULL << target;
    }
  }

  return knight_bb;
}

static Bitboard initKingAttacks(const Square sq, const Rank rank,
                                const File file) {
  Bitboard bb = 0;
  for (int dr = -1; dr <= 1; ++dr) {
    for (int df = -1; df <= 1; ++df) {
      if (dr == 0 && df == 0) {
        continue;
      }
      const int r = rank + dr;
      const int f = file + df;
      if (const bool in_bounds{r >= 0 && r < 8 && f >= 0 && f < 8})
        bb |= 1ULL << (r * 8 + f);
    }
  }
  return bb;
}

void init() {
  for (Square sq = SQ_A1; sq < SQ_NB; ++sq) {
    const auto rank = static_cast<Rank>(sq / 8);
    const auto file = static_cast<File>(sq % 8);

    pawn_attacks[WHITE][sq] = initWhitePawnAttacks(sq, rank, file);
    pawn_attacks[BLACK][sq] = initBlackPawnAttacks(sq, rank, file);
    knight_attacks[sq] = initKnightAttacks(sq, rank, file);
    king_attacks[sq] = initKingAttacks(sq, rank, file);
  }
}

} // namespace Attacks
