#include "game_engine/GameStateMachine.hpp"

GameStateMachine::GameStateMachine() = default;

GameDecision GameStateMachine::evaluate(
    const ClickContext& ctx,
    GameState currentState,
    const std::optional<Position>& currentSelectedCell) const
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
        if (!ctx.isEmpty && !ctx.isInvolved)
        {
            GameDecision decision;
            decision.action = ActionType::SelectPiece;
            decision.newState = GameState::WAITING_TARGET;
            decision.selectedCell = Position{ctx.row, ctx.col};
            return decision;
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
        if (Position{ctx.row, ctx.col} == *currentSelectedCell)
        {
            if (!ctx.isInvolved)
            {
                GameDecision decision;
                decision.action = ActionType::StartJump;
                decision.newState = GameState::JUMP_IN_PROGRESS;
                decision.from = *currentSelectedCell;
                return decision;
            }
            // התא מעורב במהלך/קפיצה — לא ניתן לקפוץ
            return {ActionType::NoOp, GameState::WAITING_TARGET};
        }

        // תרחיש 3: לחיצה על כלי ידידותי אחר — החלף בחירה
        if (ctx.hasFriendly)
        {
            GameDecision decision;
            decision.action = ActionType::SwitchPiece;
            decision.newState = GameState::WAITING_TARGET;
            decision.selectedCell = Position{ctx.row, ctx.col};
            return decision;
        }

        // תרחיש 4: מהלך לא חוקי — בטל בחירה
        if (!ctx.moveIsValid)
        {
            return {ActionType::CancelSelection, GameState::WAITING_SELECTION};
        }

        // תרחיש 5: מהלך חוקי — התחל תנועה
        {
            GameDecision decision;
            decision.action = ActionType::StartMove;
            decision.newState = GameState::MOVE_IN_PROGRESS;
            decision.from = *currentSelectedCell;
            decision.to = Position{ctx.row, ctx.col};
            return decision;
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
