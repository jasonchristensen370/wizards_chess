#include "board.h"
#include <cstdio>
#include <algorithm>
#include <cctype>

namespace Chess {

Board::Board() {
    setupInitialPosition();
}

void Board::setupInitialPosition() {
    for (auto &p: squares) p = Piece();
    // White
    for (int f=0; f<8; ++f) squares[sqidx(f,1)] = Piece(PieceType::Pawn, Color::White);
    squares[sqidx(0,0)] = Piece(PieceType::Rook, Color::White);
    squares[sqidx(7,0)] = Piece(PieceType::Rook, Color::White);
    squares[sqidx(1,0)] = Piece(PieceType::Knight, Color::White);
    squares[sqidx(6,0)] = Piece(PieceType::Knight, Color::White);
    squares[sqidx(2,0)] = Piece(PieceType::Bishop, Color::White);
    squares[sqidx(5,0)] = Piece(PieceType::Bishop, Color::White);
    squares[sqidx(3,0)] = Piece(PieceType::Queen, Color::White);
    squares[sqidx(4,0)] = Piece(PieceType::King, Color::White);
    // Black
    for (int f=0; f<8; ++f) squares[sqidx(f,6)] = Piece(PieceType::Pawn, Color::Black);
    squares[sqidx(0,7)] = Piece(PieceType::Rook, Color::Black);
    squares[sqidx(7,7)] = Piece(PieceType::Rook, Color::Black);
    squares[sqidx(1,7)] = Piece(PieceType::Knight, Color::Black);
    squares[sqidx(6,7)] = Piece(PieceType::Knight, Color::Black);
    squares[sqidx(2,7)] = Piece(PieceType::Bishop, Color::Black);
    squares[sqidx(5,7)] = Piece(PieceType::Bishop, Color::Black);
    squares[sqidx(3,7)] = Piece(PieceType::Queen, Color::Black);
    squares[sqidx(4,7)] = Piece(PieceType::King, Color::Black);

    castlingRights = 0b1111; // both sides both ways: wk,wq,bk,bq
    enpassant = -1;
    sideToMove = Color::White;
    history.clear();
}

int Board::findKing(Color c) const {
    for (int i=0;i<64;++i) if (squares[i].type==PieceType::King && squares[i].color==c) return i;
    return -1;
}

bool Board::isSquareAttacked(int sq, Color by) const {
    if (by==Color::None) return false;
    // pawns
    int r = rankOf(sq), f = fileOf(sq);
    int pawnDir = (by==Color::White)? 1 : -1;
    int pr = r - pawnDir; // to see pawn attacking sq, pawn must be on pr with diagonal files
    if (pr>=0 && pr<8) {
        if (f>0) {
            int s = sqidx(f-1, pr);
            if (squares[s].type==PieceType::Pawn && squares[s].color==by) return true;
        }
        if (f<7) {
            int s = sqidx(f+1, pr);
            if (squares[s].type==PieceType::Pawn && squares[s].color==by) return true;
        }
    }
    // knights
    const int kn[8] = {-17,-15,-10,-6,6,10,15,17};
    for (int k=0;k<8;++k) {
        int to = sq + kn[k];
        if (to<0||to>=64) continue;
        int tf=fileOf(to), tr=rankOf(to);
        if (std::abs(tf - f) > 2 || std::abs(tr - r) > 2) continue;
        if (squares[to].type==PieceType::Knight && squares[to].color==by) return true;
    }
    // sliding: bishops & queens diagonals
    const int diag[4] = {-9,-7,7,9};
    for (int d=0; d<4; ++d) {
        int offset = diag[d];
        int t = sq + offset;
        while (t>=0 && t<64) {
            int tf=fileOf(t);
            if (std::abs(tf - f)>2 && (offset==-9||offset==-7||offset==7||offset==9)) break;
            if (squares[t].type != PieceType::Empty) {
                if (squares[t].color==by && (squares[t].type==PieceType::Bishop || squares[t].type==PieceType::Queen)) return true;
                break;
            }
            t += offset;
        }
    }
    // sliding: rooks & queens
    const int orth[4] = {-8,-1,1,8};
    for (int d=0; d<4; ++d) {
        int offset = orth[d];
        int t = sq + offset;
        while (t>=0 && t<64) {
            int tf=fileOf(t);
            if (std::abs(tf - f)>4 && (offset==-1||offset==1)) break;
            if (squares[t].type != PieceType::Empty) {
                if (squares[t].color==by && (squares[t].type==PieceType::Rook || squares[t].type==PieceType::Queen)) return true;
                break;
            }
            t += offset;
        }
    }
    // king nearby
    for (int off=-9; off<=9; ++off) {
        if (off==0) continue;
        int to = sq+off;
        if (to<0||to>=64) continue;
        int tf=fileOf(to), tr=rankOf(to);
        if (std::abs(tf - f) > 1 || std::abs(tr - r) > 1) continue;
        if (squares[to].type==PieceType::King && squares[to].color==by) return true;
    }
    return false;
}

// pseudo-legal generation helpers
void Board::addPawnMoves(int sq, Color c, std::vector<Move> &out) const {
    int f = fileOf(sq), r = rankOf(sq);
    int dir = (c==Color::White) ? 1 : -1;
    int startRank = (c==Color::White) ? 1 : 6;
    int toRank = r + dir;
    if (toRank>=0 && toRank<8) {
        int to = sqidx(f,toRank);
        if (squares[to].type==PieceType::Empty) {
            // promotion?
            if ((c==Color::White && toRank==7) || (c==Color::Black && toRank==0)) {
                // promote to q,r,b,n encoded 4..1
                out.emplace_back(sq,to,4,0);
                out.emplace_back(sq,to,3,0);
                out.emplace_back(sq,to,2,0);
                out.emplace_back(sq,to,1,0);
            } else out.emplace_back(sq,to,0,0);
            // double
            if (r==startRank) {
                int to2 = sqidx(f, r + 2*dir);
                if (squares[to2].type==PieceType::Empty) out.emplace_back(sq,to2,0,0);
            }
        }
        // captures
        if (f>0) {
            int cap = sqidx(f-1,toRank);
            if (squares[cap].type!=PieceType::Empty && squares[cap].color!=c) {
                if ((c==Color::White && toRank==7) || (c==Color::Black && toRank==0)) {
                    out.emplace_back(sq,cap,4,1);
                    out.emplace_back(sq,cap,3,1);
                    out.emplace_back(sq,cap,2,1);
                    out.emplace_back(sq,cap,1,1);
                } else out.emplace_back(sq,cap,0,1);
            }
            // en-passant capture?
            if (cap == enpassant && enpassant != -1) out.emplace_back(sq,cap,0,2); // flag enpassant
        }
        if (f<7) {
            int cap = sqidx(f+1,toRank);
            if (squares[cap].type!=PieceType::Empty && squares[cap].color!=c) {
                if ((c==Color::White && toRank==7) || (c==Color::Black && toRank==0)) {
                    out.emplace_back(sq,cap,4,1);
                    out.emplace_back(sq,cap,3,1);
                    out.emplace_back(sq,cap,2,1);
                    out.emplace_back(sq,cap,1,1);
                } else out.emplace_back(sq,cap,0,1);
            }
            if (cap == enpassant && enpassant != -1) out.emplace_back(sq,cap,0,2);
        }
    }
}

void Board::addKnightMoves(int sq, Color c, std::vector<Move> &out) const {
    constexpr int offsets[8] = {-17,-15,-10,-6,6,10,15,17};
    int f=fileOf(sq), r=rankOf(sq);
    for (int i=0;i<8;++i) {
        int to = sq + offsets[i];
        if (to<0||to>=64) continue;
        int tf=fileOf(to), tr=rankOf(to);
        if (std::abs(tf - f) > 2 || std::abs(tr - r) > 2) continue;
        if (squares[to].type==PieceType::Empty || squares[to].color!=c) out.emplace_back(sq,(uint8_t)to,0,(squares[to].type==PieceType::Empty)?0:1);
    }
}

void Board::addSlidingMoves(int sq, Color c, const int *dirs, int ndirs, bool rooklike, std::vector<Move> &out) const {
    for (int d=0; d<ndirs; ++d) {
        int off = dirs[d];
        int to = sq + off;
        while (to>=0 && to<64) {
            int sf=fileOf(sq), tf=fileOf(to);
            // naive wrap guard
            if (std::abs(tf - sf) > 2 && (off==-9||off==-7||off==7||off==9)) break;
            if (squares[to].type==PieceType::Empty) out.emplace_back(sq,(uint8_t)to,0,0);
            else {
                if (squares[to].color!=c) out.emplace_back(sq,(uint8_t)to,0,1);
                break;
            }
            to += off;
        }
    }
}

void Board::addKingMoves(int sq, Color c, std::vector<Move> &out) const {
    constexpr int offs[8] = {-9,-8,-7,-1,1,7,8,9};
    for (int i=0;i<8;++i) {
        int to = sq + offs[i];
        if (to<0||to>=64) continue;
        int tf=fileOf(to), tr=rankOf(to);
        if (std::abs(tf - fileOf(sq)) > 1 || std::abs(tr - rankOf(sq)) > 1) continue;
        if (squares[to].type==PieceType::Empty || squares[to].color!=c) out.emplace_back(sq,(uint8_t)to,0,(squares[to].type==PieceType::Empty)?0:1);
    }
    // castling possibilities (pseudo-legal); legality/filtering will remove moves leaving king in check or through attacked squares
    if (c==Color::White) {
        // king side: squares f1(5), g1(6) must be empty and rights wk (bit0)
        if ((castlingRights & 1) && squares[sqidx(5,0)].type==PieceType::Empty && squares[sqidx(6,0)].type==PieceType::Empty) {
            out.emplace_back(sq,(uint8_t)sqidx(6,0),0,4); // flag castling=bit2
        }
        // queen side: d1(3), c1(2), b1(1) empty and right bit1
        if ((castlingRights & 2) && squares[sqidx(3,0)].type==PieceType::Empty && squares[sqidx(2,0)].type==PieceType::Empty && squares[sqidx(1,0)].type==PieceType::Empty) {
            out.emplace_back(sq,(uint8_t)sqidx(2,0),0,4);
        }
    } else {
        if ((castlingRights & 4) && squares[sqidx(5,7)].type==PieceType::Empty && squares[sqidx(6,7)].type==PieceType::Empty) {
            out.emplace_back(sq,(uint8_t)sqidx(6,7),0,4);
        }
        if ((castlingRights & 8) && squares[sqidx(3,7)].type==PieceType::Empty && squares[sqidx(2,7)].type==PieceType::Empty && squares[sqidx(1,7)].type==PieceType::Empty) {
            out.emplace_back(sq,(uint8_t)sqidx(2,7),0,4);
        }
    }
}

void Board::generatePseudoLegal(Color c, std::vector<Move> &out) const {
    out.clear();
    for (int i=0;i<64;++i) {
        const Piece &p = squares[i];
        if (p.type==PieceType::Empty || p.color!=c) continue;
        switch (p.type) {
            case PieceType::Pawn: addPawnMoves(i,c,out); break;
            case PieceType::Knight: addKnightMoves(i,c,out); break;
            case PieceType::Bishop: { const int d[4]={-9,-7,7,9}; addSlidingMoves(i,c,d,4,false,out); break; }
            case PieceType::Rook:   { const int d[4]={-8,-1,1,8}; addSlidingMoves(i,c,d,4,true,out); break; }
            case PieceType::Queen:  { const int d1[4]={-9,-7,7,9}; addSlidingMoves(i,c,d1,4,false,out); const int d2[4]={-8,-1,1,8}; addSlidingMoves(i,c,d2,4,true,out); break; }
            case PieceType::King: addKingMoves(i,c,out); break;
            default: break;
        }
    }
}

void Board::generateLegal(Color c, std::vector<Move> &out) {
    std::vector<Move> temp;
    generatePseudoLegal(c, temp);
    out.clear();
    out.reserve(temp.size());
    for (const Move &m : temp) {
        // make-move & check if own king is in check -> undo
        if (!makeMove(m)) { /* invalid move application */ continue; }
        int k = findKing(c);
        bool inCheck = (k==-1) ? true : isSquareAttacked(k, (c==Color::White)?Color::Black:Color::White);
        undoMove();
        if (!inCheck) out.push_back(m);
    }
}

bool Board::makeMove(const Move &m) {
    if (m.from>=64 || m.to>=64) return false;
    Undo u;
    u.mv = m;
    u.captured = squares[m.to];
    u.castlingRights = castlingRights;
    u.enpassant = enpassant;
    history.push_back(u);

    // handle en-passant capture: if flag set as enpassant (2)
    if (m.flags & 2) {
        // captured pawn is behind to square
        int capRank = rankOf(m.to) + ((sideToMove==Color::White) ? -1 : 1);
        int capSq = sqidx(fileOf(m.to), capRank);
        u.captured = squares[capSq];
        squares[capSq] = Piece();
    }

    // move piece
    Piece mover = squares[m.from];
    squares[m.to] = mover;
    squares[m.from] = Piece();

    // promotion
    if (m.promotion) {
        squares[m.to].type = static_cast<PieceType>(m.promotion + (int)PieceType::Empty); // consistent mapping
    }

    // castling: move rook accordingly
    if (m.flags & 4) {
        if (m.to == sqidx(6,0)) { // white king side
            squares[sqidx(5,0)] = squares[sqidx(7,0)];
            squares[sqidx(7,0)] = Piece();
        } else if (m.to == sqidx(2,0)) { // white queen side
            squares[sqidx(3,0)] = squares[sqidx(0,0)];
            squares[sqidx(0,0)] = Piece();
        } else if (m.to == sqidx(6,7)) { // black king side
            squares[sqidx(5,7)] = squares[sqidx(7,7)];
            squares[sqidx(7,7)] = Piece();
        } else if (m.to == sqidx(2,7)) { // black queen side
            squares[sqidx(3,7)] = squares[sqidx(0,7)];
            squares[sqidx(0,7)] = Piece();
        }
    }

    // update castling rights: if king or rook moved or rook captured
    if (mover.type==PieceType::King) {
        if (mover.color==Color::White) castlingRights &= ~(1|2);
        else castlingRights &= ~(4|8);
    } else if (mover.type==PieceType::Rook) {
        if (m.from == sqidx(0,0)) castlingRights &= ~2;
        if (m.from == sqidx(7,0)) castlingRights &= ~1;
        if (m.from == sqidx(0,7)) castlingRights &= ~8;
        if (m.from == sqidx(7,7)) castlingRights &= ~4;
    }
    // if rook captured, clear rights
    if (u.captured.type==PieceType::Rook) {
        if (m.to == sqidx(0,0)) castlingRights &= ~2;
        if (m.to == sqidx(7,0)) castlingRights &= ~1;
        if (m.to == sqidx(0,7)) castlingRights &= ~8;
        if (m.to == sqidx(7,7)) castlingRights &= ~4;
    }

    // set enpassant: if pawn double moved, set square behind pawn
    enpassant = -1;
    if (mover.type==PieceType::Pawn && std::abs((int)rankOf(m.to) - (int)rankOf(m.from))==2) {
        int epRank = (rankOf(m.to) + rankOf(m.from)) / 2;
        enpassant = sqidx(fileOf(m.from), epRank);
    }

    // change side
    sideToMove = (sideToMove==Color::White)?Color::Black:Color::White;
    return true;
}

void Board::undoMove() {
    if (history.empty()) return;
    Undo u = history.back();
    history.pop_back();
    // revert side
    sideToMove = (sideToMove==Color::White)?Color::Black:Color::White;
    // revert enpassant/castling rights
    enpassant = u.enpassant;
    castlingRights = u.castlingRights;
    // move back
    squares[u.mv.from] = squares[u.mv.to];
    // handle promotion: if promotion, revert to pawn
    if (u.mv.promotion) squares[u.mv.from].type = PieceType::Pawn;
    // restore captured piece
    if (u.mv.flags & 2) {
        // enpassant capture was captured behind 'to'
        int capRank = rankOf(u.mv.to) + ((sideToMove==Color::White) ? -1 : 1);
        int capSq = sqidx(fileOf(u.mv.to), capRank);
        squares[capSq] = u.captured;
        squares[u.mv.to] = Piece();
    } else {
        squares[u.mv.to] = u.captured;
    }
    // revert rook for castling
    if (u.mv.flags & 4) {
        if (u.mv.to == sqidx(6,0)) { // white king side
            squares[sqidx(7,0)] = squares[sqidx(5,0)];
            squares[sqidx(5,0)] = Piece();
        } else if (u.mv.to == sqidx(2,0)) { // white queen
            squares[sqidx(0,0)] = squares[sqidx(3,0)];
            squares[sqidx(3,0)] = Piece();
        } else if (u.mv.to == sqidx(6,7)) {
            squares[sqidx(7,7)] = squares[sqidx(5,7)];
            squares[sqidx(5,7)] = Piece();
        } else if (u.mv.to == sqidx(2,7)) {
            squares[sqidx(0,7)] = squares[sqidx(3,7)];
            squares[sqidx(3,7)] = Piece();
        }
    }
}

std::string Board::toString() const {
    std::string out;
    out.reserve(8*9);
    for (int r=7;r>=0;--r) {
        for (int f=0;f<8;++f) {
            const Piece &p = squares[sqidx(f,r)];
            char c='.';
            if (p.type!=PieceType::Empty) {
                switch(p.type) {
                    case PieceType::Pawn: c='P'; break;
                    case PieceType::Knight: c='N'; break;
                    case PieceType::Bishop: c='B'; break;
                    case PieceType::Rook: c='R'; break;
                    case PieceType::Queen: c='Q'; break;
                    case PieceType::King: c='K'; break;
                    default: c='?'; break;
                }
                if (p.color==Color::Black) c = std::tolower(c);
            }
            out.push_back(c);
            if (f!=7) out.push_back(' ');
        }
        out.push_back('\n');
    }
    return out;
}

void Board::debugPrint() const {
    std::string s = toString();
    std::printf("%s\n", s.c_str());
}

} // namespace Chess
