#include "Attacks.h"
#include "Move.h"
#include "Position.h"
#include "Zobrist.h"
#include "MoveGen.h"

int main() {
  Attacks::init();
  Zobrist::init();

  Position pos{};
  pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  pos.print();

  const std::vector<Move> moves{generateMoves(pos)};
  std::cout << moves.size() << '\n';

  int sq{SQ_A1};
  std::string input{};
  while (input != "x") {
    std::cout << std::flush;
    printBitboard(Attacks::bishop_mask[sq]);
    std::cout << "Square: " << sq << "\n";
    std::cin >> input;
    if (input == "b") {
      sq = std::max(0, sq - 1);
    } else {
      sq = std::min(63, sq + 1);
    }
  }

  return 0;
}
