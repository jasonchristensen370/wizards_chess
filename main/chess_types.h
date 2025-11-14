#pragma once
#include <cstdint>

namespace Chess {

enum class PieceType : uint8_t {
    Empty = 0, Pawn, Knight, Bishop, Rook, Queen, King
};

enum class Color : uint8_t {
    White = 0, Black = 1, None = 2
};

struct Piece {
    PieceType type;
    Color color;
    constexpr Piece(PieceType t = PieceType::Empty, Color c = Color::None) : type(t), color(c) {}
};

} // namespace Chess
