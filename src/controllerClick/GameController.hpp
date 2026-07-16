#pragma once

#include "game_engine/GameEngine.hpp"
#include "model/Position.hpp"

class GameController
{
public:
    explicit GameController(GameEngine& engine);

    void handleCellClick(int row, int col);
    void handleJump(int row, int col);
    void handleWait(int milliseconds);
    void handlePrint(std::ostream& output) const;

    [[nodiscard]]
    const Board& getBoard() const;

    [[nodiscard]]
    GameSnapshot getSnapshot() const;

    /// המרת פיקסלים לקואורדינטת תא (סטטית — זמינה לכולם)
    static Position pixelsToCell(int pixelX, int pixelY, int cellSize);

private:
    GameEngine& engine_;
};