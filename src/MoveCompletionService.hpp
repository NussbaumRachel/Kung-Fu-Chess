#pragma once

#include "BoardController.hpp"
#include "Board.hpp"
#include "CompletedMove.hpp"
#include "GameStateMachine.hpp"
#include "types.hpp"

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
        Position position);

private:
    BoardController& boardController_;
    Board& board_;
    GameStateMachine& stateMachine_;
};