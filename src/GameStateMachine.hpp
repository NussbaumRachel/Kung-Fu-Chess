#pragma once

#include "Position.hpp"
#include "types.hpp"
#include <optional>

enum class GameState
{
    WAITING_SELECTION,
    WAITING_TARGET,
    MOVE_IN_PROGRESS,
    JUMP_IN_PROGRESS,
    GAME_OVER
};

enum class ActionType
{
    SelectPiece,       // בחר כלי, עבור ל-WAITING_TARGET
    SwitchPiece,       // עבור לכלי ידידותי אחר
    StartMove,         // התחל תנועה, עבור ל-MOVE_IN_PROGRESS
    StartJump,         // התחל קפיצה, עבור ל-JUMP_IN_PROGRESS
    CancelSelection,   // בטל בחירה, חזור ל-WAITING_SELECTION
    NoOp               // אין פעולה (התעלם מהקליק)
};

struct GameDecision
{
    ActionType action = ActionType::NoOp;
    GameState newState = GameState::WAITING_SELECTION;
    std::optional<Position> from;
    std::optional<Position> to;
    std::optional<Position> selectedCell; // עבור SelectPiece/SwitchPiece
};

class GameStateMachine
{
public:
    GameStateMachine();

    // ── קלט: כל המידע שהמכונה צריכה, כפרמטרים ──
    // המכונה לא תלויה ב-Board, RuleEngine, או RealTimeArbiter.
    // GameEngine אוסף את המידע ומעביר אותו לכאן.
    GameDecision evaluate(
        GameState currentState,
        int clickRow, int clickCol,
        const std::optional<Position>& currentSelectedCell,
        bool cellIsEmpty,
        bool cellHasFriendlyPiece,
        bool cellIsInvolved,
        bool moveIsValid
    ) const;

    // ── ניהול מצב (state mutation — נקרא ע"י GameEngine) ──
    void setState(GameState state);
    GameState getState() const;

    void setSelectedCell(std::optional<Position> cell);
    const std::optional<Position>& getSelectedCell() const;

    void setWinner(Color color);
    const std::optional<Color>& getWinner() const;

private:
    GameState state_ = GameState::WAITING_SELECTION;
    std::optional<Position> selectedCell_;
    std::optional<Color> winner_;
};
