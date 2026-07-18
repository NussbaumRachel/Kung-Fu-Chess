#include "game_engine/ClickPreparationService.hpp"
#include "model/Board.hpp"
#include "model/Piece.hpp"
#include "arbiter/RealTimeArbiter.hpp"
#include "rule_engine/RuleEngine.hpp"
#include "game_engine/GameStateMachine.hpp"   // בשביל GameState

ClickContext ClickPreparationService::prepare(
    const Board& board,
    const RealTimeArbiter& arbiter,
    const RuleEngine& ruleEngine,
    const std::optional<Position>& selectedCell,
    GameState currentState,
    int row, int col) const
{
    ClickContext ctx;
    ctx.row = row;
    ctx.col = col;

    ctx.isEmpty = board.isEmptyCell(row, col);

    ctx.isInvolved = arbiter.isPieceInvolved(row, col);

    if (!ctx.isEmpty && selectedCell.has_value())
    {
        int sr = selectedCell->row;
        int sc = selectedCell->col;

        if (board.isInsideBoard(sr, sc) && !board.isEmptyCell(sr, sc))
        {
            Piece* selectedPiece = board.getCell(sr, sc);
            Piece* clickedPiece = board.getCell(row, col);

            if (selectedPiece && clickedPiece &&
                clickedPiece->getColor() == selectedPiece->getColor())
            {
                ctx.hasFriendly = true;
            }
        }
    }


    if (currentState == GameState::WAITING_TARGET &&
        selectedCell.has_value())
    {
        int sr = selectedCell->row;
        int sc = selectedCell->col;

        if (board.isInsideBoard(sr, sc) &&
            !board.isEmptyCell(sr, sc))
        {
            bool isSameCell = (sr == row && sc == col);
            bool isEmptyCell = ctx.isEmpty;

            if (!isSameCell && !isEmptyCell && !ctx.hasFriendly)
            {
                MoveValidation validation = ruleEngine.validateMove(
                    board,
                    Position{sr, sc},
                    Position{row, col},
                    board.getCell(sr, sc)->getColor()
                );
                ctx.moveIsValid = validation.isValid;
            }
            else if (!isSameCell && isEmptyCell)
            {
                MoveValidation validation = ruleEngine.validateMove(
                    board,
                    Position{sr, sc},
                    Position{row, col},
                    board.getCell(sr, sc)->getColor()
                );
                ctx.moveIsValid = validation.isValid;
            }
        }
    }

    return ctx;
}