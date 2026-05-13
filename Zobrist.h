#ifndef CHESS_ENGINE_ZOBRIST_H
#define CHESS_ENGINE_ZOBRIST_H
#include "types.h"

class Position;

namespace Zobrist {
void init();
Key compute(const Position &pos);

extern Key pieces[PIECE_NB][SQ_NB];
extern Key castling[CASTLING_RIGHT_NB];
extern Key en_passant[FILE_NB];
extern Key side;
}; // namespace Zobrist

#endif // CHESS_ENGINE_ZOBRIST_H
