#include "game.h"
#include <cstdio>

namespace Chess {

Game::Game() { newGame(); }

void Game::newGame() { board.setupInitialPosition(); }

void Game::debugPrintBoard() { board.debugPrint(); }

Color Game::sideToMove() const { return board.sideToMove; }

void Game::legalMoves(std::vector<Move> &out) const {
    // non-const generateLegal depends on make/undo, so create a copy
    Board tmp = board;
    const_cast<Board&>(tmp).generateLegal(board.sideToMove, out); // safe: operate on copy
}

static int fileCharToInt(char c) { return c - 'a'; }
static int rankCharToInt(char c) { return c - '1'; }

bool Game::playMoveUCI(const std::string &uci) {
    if (uci.size() < 4) return false;
    int fromF = fileCharToInt(uci[0]);
    int fromR = rankCharToInt(uci[1]);
    int toF = fileCharToInt(uci[2]);
    int toR = rankCharToInt(uci[3]);
    if (fromF<0||fromF>7||toF<0||toF>7||fromR<0||fromR>7||toR<0||toR>7) return false;
    Move m((uint8_t)(fromR*8+fromF),(uint8_t)(toR*8+toF),0,0);
    if (uci.size()>=5) {
        char p = uci[4];
        // map promotions: n=1,b=2,r=3,q=4
        if (p=='n') m.promotion = 1;
        else if (p=='b') m.promotion = 2;
        else if (p=='r') m.promotion = 3;
        else m.promotion = 4;
    }
    // Get legal moves and check if matches
    std::vector<Move> legal;
    board.generateLegal(board.sideToMove, legal);
    for (const Move &lm : legal) {
        if (lm.from==m.from && lm.to==m.to && (m.promotion==0 || m.promotion==lm.promotion)) {
            // apply the exact move (prefer move from legal list to get flags)
            board.makeMove(lm);
            return true;
        }
    }
    return false;
}

} // namespace Chess
