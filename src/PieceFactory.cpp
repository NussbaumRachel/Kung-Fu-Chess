#include "PieceFactory.hpp"
#include "King.hpp"
#include "Queen.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Knight.hpp"
#include "Pawn.hpp"

std::unique_ptr<Piece> PieceFactory::createFromToken(const std::string& token, Position startCell)
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
    case 'K': return std::make_unique<King>(color, startCell);
    case 'Q': return std::make_unique<Queen>(color, startCell);
    case 'R': return std::make_unique<Rook>(color, startCell);
    case 'B': return std::make_unique<Bishop>(color, startCell);
    case 'N': return std::make_unique<Knight>(color, startCell);
    case 'P': return std::make_unique<Pawn>(color, startCell);
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
std::string PieceFactory::makePieceToken(Color color, PieceType type)
{
    std::string token;
    token += (color == Color::White) ? 'w' : 'b';
    token += pieceTypeToChar(type);  // PieceType → 'K', 'Q', 'R' ...
    return token;
}
std::string PieceFactory::pieceTypeToChar(PieceType type)
{
    switch(type)
    {
        case PieceType::King: return "K";
        case PieceType::Queen: return "Q";
        case PieceType::Rook: return "R";
        case PieceType::Bishop: return "B";
        case PieceType::Knight: return "N";
        case PieceType::Pawn: return "P";
    }

    return "";
}
