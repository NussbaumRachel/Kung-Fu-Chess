#pragma once

#include "Board.hpp"
#include "RuleEngine.hpp"
#include "RealTimeArbiter.hpp"
#include "GameSnapshot.hpp"
#include <optional>

enum class GameState
{
    WAITING_SELECTION,
    WAITING_TARGET,
    MOVE_IN_PROGRESS,
    GAME_OVER
};

class GameEngine
{
public:
    explicit GameEngine(Board board);

    // מטעם Controller — קליק על תא בלוח
    void handleCellClick(int row, int col);

    // מטעם main loop — קידום זמן לוגי
    void advanceTime(int milliseconds);

    // מטעם Renderer — תמונת מצב אימוטאבילית
    GameSnapshot getSnapshot() const;

    GameState getState() const;

    // לטסטים
    const Board& getBoard() const;

private:
    Board board_;
    RuleEngine ruleEngine_;
    RealTimeArbiter arbiter_;

    GameState state_ = GameState::WAITING_SELECTION;
    std::optional<Position> selectedCell_;
    std::optional<Color> winner_;

    // עזרים פנימיים
    bool isPieceAtPositionInvolved(int row, int col) const;
    int calculateMoveTime(int fromRow, int fromCol,
                          int toRow, int toCol) const;
    void applyCompletedMove(const CompletedMove& cm);
};
