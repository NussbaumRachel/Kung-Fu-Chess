#include "PieceFactory.hpp"
#include "King.hpp"
#include "Queen.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Knight.hpp"
#include "Pawn.hpp"

Piece* PieceFactory::createFromToken(const std::string& token, Position startCell)
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
    case 'K': return new King(color, startCell);
    case 'Q': return new Queen(color, startCell);
    case 'R': return new Rook(color, startCell);
    case 'B': return new Bishop(color, startCell);
    case 'N': return new Knight(color, startCell);
    case 'P': return new Pawn(color, startCell);
    default:  return nullptr;
    }
}

bool PieceFactory::isValidToken(const std::string& token)
{
    if (token.length() != 2) return false;

    char color = token[0];
    if (color != 'w' && color != 'b') return false;

    char type = token[1];
    return type == 'K' || type == 'Q' || type == 'R' ||
           type == 'B' || type == 'N' || type == 'P';
}
