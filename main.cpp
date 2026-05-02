#include "Position.h"
#include <iostream>

int main() {
    Position board{};
    board.set("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2 ");
    board.print();
    return 0;
}
