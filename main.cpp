#include "Attacks.h"
#include "Move.h"
#include "MoveGen.h"
#include "Position.h"
#include "Zobrist.h"

#include <vector>

const std::string fen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int moveGenerationTest(Position &pos, int depth) {
  if (depth == 0) {
    return 1;
  }

  const std::vector<Move> moves{generateMoves(pos)};
  int num_positions{};
  for (const Move move : moves) {
    StateInfo st{};
    pos.makeMove(move, st);
    num_positions += moveGenerationTest(pos, depth - 1);
    pos.unmakeMove(move);
  }
  return num_positions;
}

int main() {
  Attacks::init();
  Zobrist::init();

  Position pos{};
  StateInfo st{};
  pos.set(fen, &st);
  pos.print();

  std::cout << moveGenerationTest(pos, 5) << std::endl;
  return 0;
}
