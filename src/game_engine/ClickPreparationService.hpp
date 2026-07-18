#pragma once

#include "game_engine/ClickContext.hpp"
#include "game_engine/GameStateMachine.hpp"   // בשביל GameState
#include "model/Position.hpp"
#include "model/Constants.hpp"
#include <optional>

class Board;
class RealTimeArbiter;
class RuleEngine;

/// SRP: כל הלוגיקה של "מה יש בתא שנלחץ" מרוכזת כאן.
/// GameEngine אוסף רפרנסים, ה-Service מעבד.
class ClickPreparationService
{
public:
    /// @param board         — הלוח (לקריאה בלבד)
    /// @param arbiter       — ארביטר (לשאילתות מעורבות)
    /// @param ruleEngine    — מנוע חוקים (לאימות מהלכים)
    /// @param selectedCell  — הכלי שנבחר כרגע (יכול להיות nullopt)
    /// @param currentState  — מצב המשחק הנוכחי
    /// @param row, col      — התא שנלחץ
    ClickContext prepare(
        const Board& board,
        const RealTimeArbiter& arbiter,
        const RuleEngine& ruleEngine,
        const std::optional<Position>& selectedCell,
        GameState currentState,
        int row, int col,
        bool isPieceResting = false) const;
};
