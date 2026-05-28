#include "Attacks.h"

#include <cstdlib>
#include <immintrin.h>

namespace Attacks {
std::array<std::array<Bitboard, SQ_NB>, COLOUR_NB> pawn_attacks{};
std::array<Bitboard, SQ_NB> knight_attacks{};
std::array<Bitboard, SQ_NB> king_attacks{};
std::array<Bitboard, SQ_NB> rook_mask{};
std::array<Bitboard, SQ_NB> bishop_mask{};
std::array<std::array<Bitboard, 4096>, SQ_NB> rook_table{};
std::array<std::array<Bitboard, 512>, SQ_NB> bishop_table{};

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

static Bitboard computeRookMask(const Square sq) {
  Bitboard mask{};
  const auto sq_rank{static_cast<Rank>(sq / 8)};
  const auto sq_file{static_cast<File>(sq % 8)};

  for (Rank rank = RANK_2; rank < RANK_8; ++rank) {
    if (rank == sq_rank) {
      continue;
    }
    mask |= 1ULL << (rank * 8 + sq_file);
  }

  for (File file = FILE_B; file < FILE_H; ++file) {
    if (file == sq_file) {
      continue;
    }
    mask |= 1ULL << (sq_rank * 8 + file);
  }

  return mask;
}

static Bitboard computeBishopMask(const Square sq) {
  Bitboard mask{};
  const auto sq_rank{static_cast<Rank>(sq / 8)};
  const auto sq_file{static_cast<File>(sq % 8)};

  for (int d = 1; d < 7; ++d) {
    const bool north_east{sq_rank + d < RANK_8 && sq_file + d < FILE_H};
    const bool south_east{sq_rank - d > RANK_1 && sq_file + d < FILE_H};
    const bool south_west{sq_rank - d > RANK_1 && sq_file - d > FILE_A};
    const bool north_west{sq_rank + d < RANK_8 && sq_file - d > FILE_A};

    if (north_east) {
      mask |= 1ULL << ((sq_rank + d) * 8 + (sq_file + d));
    }
    if (south_east) {
      mask |= 1ULL << ((sq_rank - d) * 8 + (sq_file + d));
    }
    if (south_west) {
      mask |= 1ULL << ((sq_rank - d) * 8 + (sq_file - d));
    }
    if (north_west) {
      mask |= 1ULL << ((sq_rank + d) * 8 + (sq_file - d));
    }
  }

  return mask;
}

static Bitboard computeRookAttacks(const Square sq, const Bitboard blockers) {
  Bitboard attacks = 0;
  const int sq_rank{sq / 8};
  const int sq_file{sq % 8};

  // north
  for (int d = 1; sq_rank + d < RANK_NB; ++d) {
    const Bitboard attack = 1ULL << ((sq_rank + d) * 8 + sq_file);
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }
  // south
  for (int d = 1; sq_rank - d >= RANK_1; ++d) {
    const Bitboard attack = 1ULL << ((sq_rank - d) * 8 + sq_file);
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }
  // east
  for (int d = 1; sq_file + d < FILE_NB; ++d) {
    const Bitboard attack = 1ULL << (sq_rank * 8 + (sq_file + d));
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }
  // west
  for (int d = 1; sq_file - d >= FILE_A; ++d) {
    const Bitboard attack = 1ULL << (sq_rank * 8 + (sq_file - d));
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }

  return attacks;
}

static Bitboard computeBishopAttacks(const Square sq, const Bitboard blockers) {
  Bitboard attacks = 0;
  const int sq_rank = sq / 8;
  const int sq_file = sq % 8;

  // north‑east
  for (int d = 1; sq_rank + d < RANK_NB && sq_file + d < FILE_NB; ++d) {
    const Bitboard attack = 1ULL << ((sq_rank + d) * 8 + (sq_file + d));
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }
  // north‑west
  for (int d = 1; sq_rank + d < RANK_NB && sq_file - d >= FILE_A; ++d) {
    const Bitboard attack = 1ULL << ((sq_rank + d) * 8 + (sq_file - d));
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }
  // south‑east
  for (int d = 1; sq_rank - d >= RANK_1 && sq_file + d < FILE_NB; ++d) {
    const Bitboard attack = 1ULL << ((sq_rank - d) * 8 + (sq_file + d));
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }
  // south‑west
  for (int d = 1; sq_rank - d >= RANK_1 && sq_file - d >= FILE_A; ++d) {
    const Bitboard attack = 1ULL << ((sq_rank - d) * 8 + (sq_file - d));
    attacks |= attack;
    if (blockers & attack) {
      break;
    }
  }

  return attacks;
}

void initAttackTables() {
  // rook
  for (Square sq = SQ_A1; sq < SQ_NB; ++sq) {
    const Bitboard mask = computeRookMask(sq);
    rook_mask[sq] = mask;

    Bitboard blockers = 0;
    do {
      rook_table[sq][_pext_u64(blockers, mask)] =
          computeRookAttacks(sq, blockers);
      blockers = (blockers - mask) & mask;
    } while (blockers);
  }

  // bishop
  for (Square sq = SQ_A1; sq < SQ_NB; ++sq) {
    const Bitboard mask = computeBishopMask(sq);
    bishop_mask[sq] = mask;

    Bitboard blockers = 0;
    do {
      bishop_table[sq][_pext_u64(blockers, mask)] =
          computeBishopAttacks(sq, blockers);
      blockers = (blockers - mask) & mask;
    } while (blockers);
  }
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

  initAttackTables();
}

} // namespace Attacks
