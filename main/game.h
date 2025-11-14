#pragma once
#include "board.h"
#include <vector>
#include <string>

namespace Chess {

class Game {
public:
    Game();
    void newGame();
    void debugPrintBoard();
    // play using UCI like "e2e4" or "e7e8q"
    bool playMoveUCI(const std::string &uci);
    void legalMoves(std::vector<Move> &out) const;
    Color sideToMove() const;
private:
    Board board;
};

} // namespace Chess
