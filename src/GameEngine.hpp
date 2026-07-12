#pragma once

#include "Board.hpp"
#include "RuleEngine.hpp"
#include "RealTimeArbiter.hpp"
#include "GameStateMachine.hpp"
#include "BoardController.hpp"
#include "GameSnapshot.hpp"
#include <optional>

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
    BoardController boardController_;
    GameStateMachine stateMachine_;

    static constexpr int JUMP_DURATION_MS = 1000;

    // עזרים פנימיים
    int calculateMoveTime(int fromRow, int fromCol,
                          int toRow, int toCol) const;
    void applyCompletedMove(const CompletedMove& cm);
};
