#include "rule_engine/RuleEngine.hpp"
#include "model/Board.hpp"
#include "model/Piece.hpp"

MoveValidation RuleEngine::validateMove(const Board& board,
                                        Position from, Position to,
                                        Color playerColor) const
{
    // 1. בדיקת גבולות לוח
    if (!board.isInsideBoard(from.row, from.col) ||
        !board.isInsideBoard(to.row, to.col))
    {
        return {false, "outside_board"};
    }

    // 2. בדיקה שיש כלי במקור
    if (board.isEmptyCell(from.row, from.col))
    {
        return {false, "empty_source"};
    }

    Piece* piece = board.getCell(from.row, from.col);

    // 3. בדיקה שהכלי שייך לשחקן
    if (piece->getColor() != playerColor)
    {
        return {false, "empty_source"};
    }

    // 4. בדיקת חוקיות תנועה לפי סוג הכלי
    if (!piece->isValidMove(from.row, from.col, to.row, to.col, board))
    {
        return {false, "illegal_piece_move"};
    }

    // 5. בדיקת מסלול פנוי (פרט לפרש)
    if (piece->getType() != PieceType::Knight)
    {
        if (!board.isPathClear(from.row, from.col, to.row, to.col))
        {
            return {false, "illegal_piece_move"};
        }
    }

    // 6. בדיקת כלי ידידותי ביעד
    if (board.hasFriendlyPiece(to.row, to.col, playerColor))
    {
        return {false, "friendly_destination"};
    }

    return {true, "ok"};
}
