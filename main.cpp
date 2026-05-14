#include "Attacks.h"
#include "Move.h"
#include "Position.h"
#include "Zobrist.h"

int main() {
  Attacks::init();
  Zobrist::init();

  Position board{};
  board.set("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
  board.print();

  std::cout << board.fen() << '\n';
  constexpr Move m{SQ_H2, SQ_H3, QUIET};
  std::cout << static_cast<int>(m.from()) << " " << static_cast<int>(m.to())
            << " " << m.flags() << '\n';

  board.makeMove(m);

  board.print();
  printBitboard(Attacks::knight_attacks[SQ_E5]);
  return 0;
}
