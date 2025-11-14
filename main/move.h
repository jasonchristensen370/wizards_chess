#pragma once
#include <cstdint>
#include <string>

namespace Chess {

struct Move {
    uint8_t from;      // 0..63
    uint8_t to;        // 0..63
    uint8_t promotion; // 0 = none, else PieceType as integer
    uint8_t flags;     // bitflags: bit0 = capture, bit1 = enpassant, bit2 = castling
    Move(uint8_t f=0,uint8_t t=0,uint8_t p=0,uint8_t fl=0) : from(f), to(t), promotion(p), flags(fl) {}
};

inline std::string moveToUCI(const Move &m) {
    auto sq = [](int s)->std::string {
        char buf[3];
        buf[0] = 'a' + (s & 7);
        buf[1] = '1' + (s >> 3);
        buf[2] = '\0';
        return std::string(buf);
    };
    std::string s = sq(m.from) + sq(m.to);
    if (m.promotion) {
        char pc='q';
        switch((int)m.promotion){
            case 1: pc='n'; break;
            case 2: pc='b'; break;
            case 3: pc='r'; break;
            case 4: pc='q'; break;
            default: pc='q'; break;
        }
        s.push_back(pc);
    }
    return s;
}

} // namespace Chess
