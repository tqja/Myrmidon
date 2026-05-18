#include "MoveGen.h"
#include "Attacks.h"
#include <vector>

static void generatePromos(std::vector<Move> &move_list, const Square from,
                           const Square single_push) {
  for (MoveFlags promo :
       {KNIGHT_PROMO, BISHOP_PROMO, ROOK_PROMO, QUEEN_PROMO}) {
    move_list.emplace_back(from, single_push, promo);
  }
}
static void generatePromoCaptures(std::vector<Move> &move_list,
                                  const Square from, const Square to) {
  for (MoveFlags promo : {KNIGHT_PROMO_CAPTURE, BISHOP_PROMO_CAPTURE,
                          ROOK_PROMO_CAPTURE, QUEEN_PROMO_CAPTURE}) {
    move_list.emplace_back(from, to, promo);
  }
}
static void generateEnPassant(const Position &pos, std::vector<Move> &move_list,
                              const Colour ally, const Colour enemy) {
  if (pos.getEp() != SQ_NONE) {
    const Square ep{pos.getEp()};
    const Bitboard ally_pawns{pos.getPieces(PAWN, ally)};
    Bitboard ep_attackers{Attacks::pawn_attacks[enemy][ep] & ally_pawns};
    while (ep_attackers) {
      const Square from{popLsb(ep_attackers)};
      move_list.emplace_back(from, ep, EP_CAPTURE);
    }
  }
}

static void generatePawnMoves(const Position &pos, std::vector<Move> &move_list,
                              const Bitboard enemy_pieces, const Colour ally,
                              const Colour enemy) {
  Bitboard pawns{pos.getPieces(PAWN, ally)};
  const int push_direction{ally == WHITE ? NORTH : SOUTH};
  const Rank start_rank{ally == WHITE ? RANK_2 : RANK_7};

  while (pawns) {
    const Square from{popLsb(pawns)};
    const Rank promo_rank{ally == WHITE ? RANK_8 : RANK_1};

    Bitboard capture_targets{Attacks::pawn_attacks[ally][from] & enemy_pieces};
    while (capture_targets) {
      const Square to{popLsb(capture_targets)};
      if (to / 8 == promo_rank) {
        generatePromoCaptures(move_list, from, to);
      } else {
        move_list.emplace_back(from, to, CAPTURE);
      }
    }

    const auto rank{static_cast<Rank>(from / 8)};
    assert(!(ally == WHITE && rank == RANK_8) &&
           !(ally == BLACK && rank == RANK_1) &&
           "Invalid position: Pawn cannot exist on final rank");

    const auto single_push{static_cast<Square>(from + push_direction)};
    if (!pos.empty(single_push))
      continue;

    if (rank + push_direction == promo_rank) {
      generatePromos(move_list, from, single_push);
    } else {
      move_list.emplace_back(from, single_push, QUIET);
    }

    if (rank != start_rank)
      continue;

    const auto double_push{static_cast<Square>(single_push + push_direction)};
    if (pos.empty(double_push)) {
      move_list.emplace_back(from, double_push, DOUBLE_PAWN);
    }
  }

  generateEnPassant(pos, move_list, ally, enemy);
}

static void generateKnightMoves(const Position &pos,
                                std::vector<Move> &move_list,
                                const Bitboard ally_pieces,
                                const Bitboard enemy_pieces) {
  Bitboard knights{pos.getPieces(KNIGHT) & ally_pieces};
  while (knights) {
    const Square from{popLsb(knights)};
    Bitboard targets{Attacks::knight_attacks[from] & ~ally_pieces};
    while (targets) {
      const Square to{popLsb(targets)};
      const MoveFlags flags{(enemy_pieces & (1ULL << to)) ? CAPTURE : QUIET};
      move_list.emplace_back(from, to, flags);
    }
  }
}

static void generateBishopMoves(const Position &pos,
                                std::vector<Move> &move_list,
                                const Bitboard ally_pieces,
                                const Bitboard enemy_pieces,
                                const Bitboard occupancy) {
  Bitboard bishops{pos.getPieces(BISHOP) & ally_pieces};
  while (bishops) {
    const Square from{popLsb(bishops)};
    Bitboard targets{Attacks::bishop_attacks(from, occupancy) & ~ally_pieces};
    while (targets) {
      const Square to{popLsb(targets)};
      const MoveFlags flags{(enemy_pieces & (1ULL << to)) ? CAPTURE : QUIET};
      move_list.emplace_back(from, to, flags);
    }
  }
}

static void generateRookMoves(const Position &pos, std::vector<Move> &move_list,
                              const Bitboard ally_pieces,
                              const Bitboard enemy_pieces,
                              const Bitboard occupancy) {
  Bitboard rook{pos.getPieces(ROOK) & ally_pieces};
  while (rook) {
    const Square from{popLsb(rook)};
    Bitboard targets{Attacks::rook_attacks(from, occupancy) & ~ally_pieces};
    while (targets) {
      const Square to{popLsb(targets)};
      const MoveFlags flags{(enemy_pieces & (1ULL << to)) ? CAPTURE : QUIET};
      move_list.emplace_back(from, to, flags);
    }
  }
}

static void generateQueenMoves(const Position &pos,
                               std::vector<Move> &move_list,
                               const Bitboard ally_pieces,
                               const Bitboard enemy_pieces,
                               const Bitboard occupancy) {
  Bitboard queen{pos.getPieces(QUEEN) & ally_pieces};
  while (queen) {
    const Square from{popLsb(queen)};
    Bitboard targets{Attacks::queen_attacks(from, occupancy) & ~ally_pieces};
    while (targets) {
      const Square to{popLsb(targets)};
      const MoveFlags flags{(enemy_pieces & (1ULL << to)) ? CAPTURE : QUIET};
      move_list.emplace_back(from, to, flags);
    }
  }
}

static void generateKingMoves(const Position &pos, std::vector<Move> &move_list,
                              const Bitboard ally_pieces,
                              const Bitboard enemy_pieces) {
  Bitboard kings{pos.getPieces(KING) & ally_pieces};
  while (kings) {
    const Square from{popLsb(kings)};
    Bitboard targets{Attacks::king_attacks[from] & ~ally_pieces};
    while (targets) {
      const Square to{popLsb(targets)};
      const MoveFlags flags{(enemy_pieces & (1ULL << to)) ? CAPTURE : QUIET};
      move_list.emplace_back(from, to, flags);
    }
  }
}

static void generateCastlingMoves(const Position &pos,
                                  std::vector<Move> &move_list,
                                  const Colour ally, const Colour enemy) {
  const CastlingRights cr = pos.getCastleRights();

  if (ally == WHITE) {
    constexpr Square king_sq{SQ_E1};
    if (pos.isSquareAttacked(king_sq, enemy)) {
      return;
    };

    const bool queenside = (cr & WHITE_OOO) && pos.empty(SQ_B1) &&
                           pos.empty(SQ_C1) && pos.empty(SQ_D1) &&
                           !pos.isSquareAttacked(SQ_C1, enemy) &&
                           !pos.isSquareAttacked(SQ_D1, enemy);
    const bool kingside = (cr & WHITE_OO) && pos.empty(SQ_F1) &&
                          pos.empty(SQ_G1) &&
                          !pos.isSquareAttacked(SQ_F1, enemy) &&
                          !pos.isSquareAttacked(SQ_G1, enemy);

    if (queenside) {
      move_list.emplace_back(SQ_E1, SQ_C1, QUEEN_CASTLE);
    }
    if (kingside) {
      move_list.emplace_back(SQ_E1, SQ_G1, KING_CASTLE);
    }

  } else {
    constexpr Square king_sq = SQ_E8;
    if (pos.isSquareAttacked(king_sq, enemy)) {
      return;
    }

    const bool can_castle_queenside = (cr & BLACK_OOO) && pos.empty(SQ_B8) && pos.empty(SQ_C8) &&
                                      pos.empty(SQ_D8) &&
                                      !pos.isSquareAttacked(SQ_C8, enemy) &&
                                      !pos.isSquareAttacked(SQ_D8, enemy);

    const bool can_castle_kingside = (cr & BLACK_OO) && pos.empty(SQ_F8) &&
                                     pos.empty(SQ_G8) &&
                                     !pos.isSquareAttacked(SQ_F8, enemy) &&
                                     !pos.isSquareAttacked(SQ_G8, enemy);

    if (can_castle_queenside) {
      move_list.emplace_back(SQ_E8, SQ_C8, QUEEN_CASTLE);
    }
    if (can_castle_kingside) {
      move_list.emplace_back(SQ_E8, SQ_G8, KING_CASTLE);
    }
  }
}

std::vector<Move> generateMoves(const Position &pos) {
  std::vector<Move> move_list{};
  const Colour ally{pos.getSide()};
  const Colour enemy{opposite(ally)};
  const Bitboard ally_pieces{pos.getPieces(ally)};
  const Bitboard enemy_pieces{pos.getPieces(enemy)};
  const Bitboard occupancy{pos.getPieces()};

  generatePawnMoves(pos, move_list, enemy_pieces, ally, enemy);
  generateKnightMoves(pos, move_list, ally_pieces, enemy_pieces);
  generateBishopMoves(pos, move_list, ally_pieces, enemy_pieces, occupancy);
  generateRookMoves(pos, move_list, ally_pieces, enemy_pieces, occupancy);
  generateQueenMoves(pos, move_list, ally_pieces, enemy_pieces, occupancy);
  generateKingMoves(pos, move_list, ally_pieces, enemy_pieces);
  generateCastlingMoves(pos, move_list, ally, enemy);

  return move_list;
}
