#include "game_engine/BoardController.hpp"
#include "movement/PieceFactory.hpp"
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

Piece* BoardController::promotePiece(Piece* piece, Position at, PieceType newType)
{
    Color pieceColor = piece->getColor();

    // מוודאים שזה הכלי מהלוח, ולוקחים אותו החוצה (unique_ptr ממנו הוא נמחק אוטומטית)
    Piece* existing = board_.getCell(at.row, at.col);
    assert(existing == piece);
    board_.takeCell(at.row, at.col);  // unique_ptr שחוזר נהרס כאן → delete piece

    // יצירת כלי חדש על הלוח
    auto newPiece = PieceFactory::createFromToken(
        PieceFactory::makePieceToken(pieceColor, newType),
        at
    );
    Piece* newPieceRaw = newPiece.get();
    board_.setCell(at.row, at.col, std::move(newPiece));

    return newPieceRaw;  // observing pointer — בעלות ב-Board
}