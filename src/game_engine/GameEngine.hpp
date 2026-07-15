#pragma once
#include "game_engine/MoveCompletionService.hpp"
#include "model/Board.hpp"
#include "rule_engine/RuleEngine.hpp"
#include "arbiter/RealTimeArbiter.hpp"
#include "game_engine/GameStateMachine.hpp"
#include "game_engine/BoardController.hpp"
#include "game_engine/GameSnapshot.hpp"
#include <optional>

class GameEngine
{
public:
    explicit GameEngine(Board board);

    // מטעם Controller — קליק על תא בלוח
    void handleCellClick(int row, int col);

    // מטעם Controller — קפיצה ישירה (ללא two-click flow)
    void handleJump(int row, int col);

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
    MoveCompletionService moveCompletionService_;
    static constexpr int JUMP_DURATION_MS = 1000;

    // עזרים פנימיים
    int calculateMoveTime(int fromRow, int fromCol,
                          int toRow, int toCol) const;
    // void applyCompletedMove(const CompletedMove& cm);
};
