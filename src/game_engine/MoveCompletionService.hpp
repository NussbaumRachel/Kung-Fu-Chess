#pragma once

#include "game_engine/BoardController.hpp"
#include "model/Board.hpp"
#include "movement/CompletedMove.hpp"
#include "game_engine/GameStateMachine.hpp"
#include "model/Constants.hpp"

#include <optional>

struct MoveCompletionResult
{
    bool gameOver = false;
    std::optional<Color> winner;
};


class MoveCompletionService
{
public:
    MoveCompletionService(
        BoardController& boardController,
        Board& board,
        GameStateMachine& stateMachine);

    MoveCompletionResult completeMove(
        const CompletedMove& completedMove);

private:
    bool isKing(const Piece* piece) const;

    void handlePromotion(
        Piece* piece,
        Position position,
        PieceType promoteTo = PieceType::Queen);

private:
    BoardController& boardController_;
    Board& board_;
    GameStateMachine& stateMachine_;
};