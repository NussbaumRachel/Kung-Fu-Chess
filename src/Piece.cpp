#include "Piece.hpp"
#include "King.hpp"
#include "Queen.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Knight.hpp"
#include "Pawn.hpp"
#include "Board.hpp"

Piece::Piece(Color color, PieceType type)
    : color_(color), type_(type)
{
}

Color Piece::getColor() const { return color_; }
PieceType Piece::getType() const { return type_; }

bool Piece::isValidToken(const std::string& token)
{
    return createFromToken(token) != nullptr;
}

std::string Piece::toString() const
{
    std::string result;
    result += (color_ == Color::White) ? 'w' : 'b';
    switch (type_)
    {
    case PieceType::King:   result += 'K'; break;
    case PieceType::Queen:  result += 'Q'; break;
    case PieceType::Rook:   result += 'R'; break;
    case PieceType::Bishop: result += 'B'; break;
    case PieceType::Knight: result += 'N'; break;
    case PieceType::Pawn:   result += 'P'; break;
    }
    return result;
}

Piece* Piece::createFromToken(const std::string& token)
{
    if (token.length() != 2) return nullptr;

    Color color;
    switch (token[0])
    {
    case 'w': color = Color::White; break;
    case 'b': color = Color::Black; break;
    default:  return nullptr;
    }

    switch (token[1])
    {
    case 'K': return new King(color);
    case 'Q': return new Queen(color);
    case 'R': return new Rook(color);
    case 'B': return new Bishop(color);
    case 'N': return new Knight(color);
    case 'P': return new Pawn(color);
    default:  return nullptr;
    }
}
