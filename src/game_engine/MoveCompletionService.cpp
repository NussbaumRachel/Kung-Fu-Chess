#include "game_engine/MoveCompletionService.hpp"
#include <cassert>


MoveCompletionService::MoveCompletionService(
    BoardController& boardController,
    Board& board,
    GameStateMachine& stateMachine)
    :
    boardController_(boardController),
    board_(board),
    stateMachine_(stateMachine)
{
}


MoveCompletionResult MoveCompletionService::completeMove(
    const CompletedMove& completedMove)
{
    if (completedMove.wasCancelled)
    {
        if (completedMove.wasIntercepted)
        {
            // קופץ יירט את הכלי — מסירים מהלוח
            // unique_ptr שחוזר מ-takeCell נהרס אוטומטית ביציאה מה-scope
            board_.takeCell(
                completedMove.from.row,
                completedMove.from.col
            );
        }
        return {};
    }

    MoveCompletionResult result;

    Piece* piece = board_.getCell(
        completedMove.from.row,
        completedMove.from.col
    );

    if (piece != completedMove.piece)
        return result;

    Piece* target = board_.getCell(
        completedMove.to.row,
        completedMove.to.col
    );

    bool targetIsKing = isKing(target);

    // executeMove מחזיר unique_ptr — captured ימחק אוטומטית בסוף
    std::unique_ptr<Piece> captured = boardController_.executeMove(
        *completedMove.piece,
        completedMove.from,
        completedMove.to
    );
    if (captured) {
    result.pointsAwarded = pieceValue(captured->getType());
    result.scoringColor = completedMove.piece->getColor(); // the capturer
    }
    if (targetIsKing)
    {
        // captured (שהוא ה-King) ימחק אוטומטית כאן
        result.gameOver = true;
        result.winner = completedMove.piece->getColor();
        return result;
    }

    // captured ימחק אוטומטית ביציאה

    Piece* movingPiece = board_.getCell(
        completedMove.to.row,
        completedMove.to.col
    );
    handlePromotion(
        movingPiece,
        completedMove.to,
        completedMove.promoteTo.value_or(PieceType::Queen)
    );

    return result;
}


bool MoveCompletionService::isKing(const Piece* piece) const
{
    return piece != nullptr &&
           piece->getType() == PieceType::King;
}


void MoveCompletionService::handlePromotion(
    Piece* piece,
    Position position,
    PieceType promoteTo)
{
    if (!piece)
        return;

    if (piece->getType() != PieceType::Pawn)
        return;

    int lastRow =
        (piece->getColor() == Color::White)
        ? 0
        : board_.rowCount() - 1;

    if (position.row == lastRow)
    {
        boardController_.promotePiece(piece, position, promoteTo);
    }
}