#ifndef CHESS_ENGINE_ZOBRIST_H
#define CHESS_ENGINE_ZOBRIST_H
#include <cstdint>

#include "Position.h"
#include "types.h"

class Zobrist {
public:
  Zobrist();
  std::uint64_t compute(const Position &pos) const;

private:
  Key pieces[PIECE_NB][SQ_NB]{};
  Key castling[CASTLING_RIGHT_NB]{};
  Key en_passant[FILE_NB]{};
  Key side{};
};

#endif // CHESS_ENGINE_ZOBRIST_H
