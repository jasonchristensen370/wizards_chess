#pragma once
#include "chess_types.h"
#include "move.h"
#include <array>
#include <vector>
#include <string>

namespace Chess {

struct Undo {
    Move mv;
    Piece captured;
    uint8_t castlingRights; // 4 bits: wk, wq, bk, bq
    int8_t enpassant; // -1 none or square
};

class Board {
public:
    Board();
    void setupInitialPosition();
    // generate pseudo-legal moves for color
    void generatePseudoLegal(Color c, std::vector<Move> &out) const;
    // generate legal moves (filters pseudo-legal by not leaving king in check)
    void generateLegal(Color c, std::vector<Move> &out);
    // make and undo
    bool makeMove(const Move &m);
    void undoMove();
    // helpers
    bool isSquareAttacked(int sq, Color by) const;
    int findKing(Color c) const;
    std::string toString() const;
    void debugPrint() const;

    // state
    std::array<Piece,64> squares;
    uint8_t castlingRights; // bits: 0 white king,1 white queen,2 black king,3 black queen
    int8_t enpassant; // square index or -1
    Color sideToMove;
    std::vector<Undo> history; // small vector for undo stack

private:
    static constexpr int fileOf(int sq) { return sq & 7; }
    static constexpr int rankOf(int sq) { return sq >> 3; }
    static int sqidx(int f,int r) { return r*8 + f; }

    void addPawnMoves(int sq, Color c, std::vector<Move> &out) const;
    void addKnightMoves(int sq, Color c, std::vector<Move> &out) const;
    void addKingMoves(int sq, Color c, std::vector<Move> &out) const;
    void addSlidingMoves(int sq, Color c, const int *dirs, int ndirs, bool rooklike, std::vector<Move> &out) const;
};

} // namespace Chess
