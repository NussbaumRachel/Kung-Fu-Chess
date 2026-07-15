#pragma once

#include "game_engine/GameEngine.hpp"
#include "model/Position.hpp"

class GameController
{
public:
    explicit GameController(GameEngine& engine, int cellSize = 100);

    void handleClick(int pixelX, int pixelY);
    void handleJump(int pixelX, int pixelY);
    void handleWait(int milliseconds);
    void handlePrint(std::ostream& output) const;

    [[nodiscard]]
    const Board& getBoard() const;

    [[nodiscard]]
    GameSnapshot getSnapshot() const;

private:
    [[nodiscard]]
    Position pixelsToCell(int pixelX, int pixelY) const;

private:
    GameEngine& engine_;
    int cellSize_;
};