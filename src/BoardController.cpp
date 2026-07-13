#include "BoardController.hpp"
#include "PieceFactory.hpp"
#include <cassert>

BoardController::BoardController(Board& board)
    : board_(board)
{
}

std::unique_ptr<Piece> BoardController::executeMove(Piece& piece, Position from, Position to)
{
    // שליפת הכלי מהמקור — מוודאים שזה באמת ה-piece הנכון
    std::unique_ptr<Piece> removed = board_.takeCell(from.row, from.col);
    assert(removed.get() == &piece);

    // בדיקה אם יש כלי ביעד — לוכדים אותו
    Piece* capturedRaw = board_.getCell(to.row, to.col);
    std::unique_ptr<Piece> capturedPiece;
    if (capturedRaw)
    {
        capturedRaw->setState(PieceState::Captured);
        capturedPiece = board_.takeCell(to.row, to.col);
    }

    // מיקום הכלי ביעד
    board_.setCell(to.row, to.col, std::move(removed));
    piece.setCell(to);
    piece.setState(PieceState::Idle);

    return capturedPiece;
}

Piece* BoardController::promoteToQueen(Piece* pawn, Position at)
{
    Color pawnColor = pawn->getColor();

    // מוודאים שזה החייל מהלוח, ולוקחים אותו החוצה (unique_ptr ממנו הוא נמחק אוטומטית)
    Piece* existing = board_.getCell(at.row, at.col);
    assert(existing == pawn);
    board_.takeCell(at.row, at.col);  // unique_ptr שחוזר נהרס כאן → delete pawn

    // יצירת מלכה חדשה על הלוח
    auto queen = PieceFactory::createFromToken(
        PieceFactory::makePieceToken(pawnColor, PieceType::Queen),
        at
    );
    Piece* queenRaw = queen.get();
    board_.setCell(at.row, at.col, std::move(queen));

    return queenRaw;  // observing pointer — בעלות ב-Board
}