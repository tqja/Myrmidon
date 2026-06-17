#include "MoveGen.h"
#include "Position.h"
#include "types.h"

inline int perft(Position &pos, const int depth) {

  if (depth == 0) {
    return 1;
  }

  const std::vector<Move> moves{generateMoves(pos)};
  int nodes{};

  for (const Move move : moves) {
    Colour ally{pos.getSide()};
    StateInfo st{};
    pos.makeMove(move, st);

    if (!pos.kingLeftInCheck()) {
      nodes += perft(pos, depth - 1);
    }

    pos.unmakeMove(move);
  }

  return nodes;
}

inline int perft(const std::string fen, const int depth) {
  StateInfo st{};
  Position pos{};
  pos.set(fen, &st);
  return perft(pos, depth);
}
