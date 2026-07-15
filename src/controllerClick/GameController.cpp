#include "controllerClick/GameController.hpp"

GameController::GameController(GameEngine& engine, int cellSize)
    : engine_(engine),
      cellSize_(cellSize)
{
}


void GameController::handleClick(int pixelX, int pixelY)
{
    Position cell = pixelsToCell(pixelX, pixelY);

    engine_.handleCellClick(
        cell.row,
        cell.col
    );
}


void GameController::handleJump(int pixelX, int pixelY)
{
    Position cell = pixelsToCell(pixelX, pixelY);

    engine_.handleJump(
        cell.row,
        cell.col
    );
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


Position GameController::pixelsToCell(int pixelX, int pixelY) const
{
    return Position{
        pixelY / cellSize_,
        pixelX / cellSize_
    };
}
void GameController::handlePrint(std::ostream& output) const
{
    engine_.getBoard().print();
}