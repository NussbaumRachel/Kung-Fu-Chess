#ifndef TYPES_H
#define TYPES_H

enum class Color
{
    White,
    Black
};

enum class PieceType
{
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};
inline constexpr int pieceValue(PieceType type) {
    switch (type) {
        case PieceType::Pawn:   return 1;
        case PieceType::Knight: return 3;
        case PieceType::Bishop: return 3;
        case PieceType::Rook:   return 5;
        case PieceType::Queen:  return 9;
        case PieceType::King:   return 0;
    }
    return 0;
}

#endif
