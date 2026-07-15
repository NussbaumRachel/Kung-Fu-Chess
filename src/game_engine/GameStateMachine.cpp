#include "game_engine/GameStateMachine.hpp"

GameStateMachine::GameStateMachine() = default;

GameDecision GameStateMachine::evaluate(
    GameState currentState,
    int clickRow, int clickCol,
    const std::optional<Position>& currentSelectedCell,
    bool cellIsEmpty,
    bool cellHasFriendlyPiece,
    bool cellIsInvolved,
    bool moveIsValid) const
{
    // ── GAME_OVER: אין קליקים ──
    if (currentState == GameState::GAME_OVER)
    {
        return {ActionType::NoOp, GameState::GAME_OVER};
    }

    // ── WAITING_SELECTION / MOVE_IN_PROGRESS / JUMP_IN_PROGRESS: בחירת כלי ──
    if (currentState == GameState::WAITING_SELECTION ||
        currentState == GameState::MOVE_IN_PROGRESS ||
        currentState == GameState::JUMP_IN_PROGRESS)
    {
        // ניתן לבחור רק כלי פנוי (לא ריק, לא בתנועה/קפיצה)
        if (!cellIsEmpty && !cellIsInvolved)
        {
            GameDecision d;
            d.action = ActionType::SelectPiece;
            d.newState = GameState::WAITING_TARGET;
            d.selectedCell = Position{clickRow, clickCol};
            return d;
        }
        return {ActionType::NoOp, currentState};
    }

    // ── WAITING_TARGET: ביצוע פעולה על בסיס התא שנלחץ ──
    if (currentState == GameState::WAITING_TARGET)
    {
        // תרחיש 1: הכלי הנבחר נעלם (נלכד/לא קיים) — בטל בחירה
        if (!currentSelectedCell.has_value())
        {
            return {ActionType::CancelSelection, GameState::WAITING_SELECTION};
        }

        // תרחיש 2: לחיצה על אותו תא — קפיצה
        if (Position{clickRow, clickCol} == *currentSelectedCell)
        {
            if (!cellIsInvolved)
            {
                GameDecision d;
                d.action = ActionType::StartJump;
                d.newState = GameState::JUMP_IN_PROGRESS;
                d.from = *currentSelectedCell;
                return d;
            }
            // התא מעורב במהלך/קפיצה — לא ניתן לקפוץ
            return {ActionType::NoOp, GameState::WAITING_TARGET};
        }

        // תרחיש 3: לחיצה על כלי ידידותי אחר — החלף בחירה
        if (cellHasFriendlyPiece)
        {
            GameDecision d;
            d.action = ActionType::SwitchPiece;
            d.newState = GameState::WAITING_TARGET;
            d.selectedCell = Position{clickRow, clickCol};
            return d;
        }

        // תרחיש 4: מהלך לא חוקי — בטל בחירה
        if (!moveIsValid)
        {
            return {ActionType::CancelSelection, GameState::WAITING_SELECTION};
        }

        // תרחיש 5: מהלך חוקי — התחל תנועה
        {
            GameDecision d;
            d.action = ActionType::StartMove;
            d.newState = GameState::MOVE_IN_PROGRESS;
            d.from = *currentSelectedCell;
            d.to = Position{clickRow, clickCol};
            return d;
        }
    }

    return {ActionType::NoOp, currentState};
}

// ── ניהול מצב ──

void GameStateMachine::setState(GameState state)
{
    state_ = state;
}

GameState GameStateMachine::getState() const
{
    return state_;
}

void GameStateMachine::setSelectedCell(std::optional<Position> cell)
{
    selectedCell_ = cell;
}

const std::optional<Position>& GameStateMachine::getSelectedCell() const
{
    return selectedCell_;
}

void GameStateMachine::setWinner(Color color)
{
    winner_ = color;
}

const std::optional<Color>& GameStateMachine::getWinner() const
{
    return winner_;
}
