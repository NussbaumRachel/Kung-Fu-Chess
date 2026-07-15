#include "model/Piece.hpp"
#include "model/Board.hpp"

int Piece::nextId_ = 1;

Piece::Piece(Color color, PieceType type, Position startCell)
    : id_(nextId_++), color_(color), type_(type), cell_(startCell)
{
}

int Piece::getId() const { return id_; }
Color Piece::getColor() const { return color_; }
PieceType Piece::getType() const { return type_; }
PieceState Piece::getState() const { return state_; }
Position Piece::getCell() const { return cell_; }

void Piece::setState(PieceState state) { state_ = state; }
void Piece::setCell(Position cell) { cell_ = cell; }

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
