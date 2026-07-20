#include "controllerClick/GameController.hpp"

GameController::GameController(GameEngine& engine)
    : engine_(engine)
{
}

void GameController::handleCellClick(int row, int col)
{
    engine_.handleCellClick(row, col);
}

void GameController::handleJump(int row, int col)
{
    engine_.handleJump(row, col);
}

void GameController::handleWait(int milliseconds)
{
    engine_.advanceTime(milliseconds);
}

const Board& GameController::getBoard() const
{
    return engine_.getBoard();
}

GameSnapshot GameController::getSnapshot() const
{
    return engine_.getSnapshot();
}

Position GameController::pixelsToCell(int pixelX, int pixelY, int cellSize)
{
    return Position{
        pixelY / cellSize,
        pixelX / cellSize
    };
}


void GameController::handlePrint(std::ostream& output) const
{
    engine_.getBoard().print();
}