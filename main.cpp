#include "Position.h"
#include "Zobrist.h"
#include "Move.h"

int main() {
    Position board{};
    board.set("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
    board.print();

    std::cout << "Hash:              " << Zobrist::compute(board) << '\n';
    std::cout << board.fen() << '\n';
    constexpr Move m{SQ_E4, SQ_E6, QUEEN_CASTLE};
    std::cout << static_cast<int>(m.from()) << " " << static_cast<int>(m.to()) << " " << m.flags() << '\n';
    return 0;
}
