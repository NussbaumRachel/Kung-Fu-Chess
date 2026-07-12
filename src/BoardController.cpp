#include "BoardController.hpp"
#include "PieceFactory.hpp"

BoardController::BoardController(Board& board)
    : board_(board)
{
}

Piece* BoardController::executeMove(Piece* piece, Position from, Position to)
{
    // שליפת הכלי מהמקור
    board_.takeCell(from.row, from.col);

    // בדיקה אם יש כלי ביעד
    Piece* capturedPiece = board_.getCell(to.row, to.col);
    if (capturedPiece)
    {
        capturedPiece->setState(PieceState::Captured);
        board_.takeCell(to.row, to.col);
        delete capturedPiece;
    }

    // מיקום הכלי ביעד
    board_.setCell(to.row, to.col, piece);
    piece->setCell(to);
    piece->setState(PieceState::Idle);

    return capturedPiece;
}

bool BoardController::isKingAt(Position pos) const
{
    Piece* piece = board_.getCell(pos.row, pos.col);
    return piece != nullptr && piece->getType() == PieceType::King;
}

Piece* BoardController::promoteToQueen(Piece* pawn, Position at)
{
    // שמירת הצבע לפני מחיקת החייל
    Color pawnColor = pawn->getColor();

    // מחיקת החייל מהלוח
    delete board_.takeCell(at.row, at.col);

    // יצירת מלכה חדשה
    std::string token;
    token += (pawnColor == Color::White) ? 'w' : 'b';
    token += 'Q';

    Piece* queen = PieceFactory::createFromToken(token, at);
    board_.setCell(at.row, at.col, queen);

    return queen;
}
