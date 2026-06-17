#include "Attacks.h"
#include "Position.h"
#include "Zobrist.h"
#include "perft.h"

const std::string fen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";

int main() {
  Attacks::init();
  Zobrist::init();

  Position pos{};
  StateInfo st{};
  pos.set(fen, &st);
  pos.print();

  std::cout << perft(pos, 4) << '\n';

  return 0;
}
