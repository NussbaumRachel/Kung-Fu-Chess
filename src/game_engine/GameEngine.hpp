#pragma once
#include "game_engine/MoveCompletionService.hpp"
#include "model/Board.hpp"
#include "rule_engine/RuleEngine.hpp"
#include "arbiter/RealTimeArbiter.hpp"
#include "game_engine/GameStateMachine.hpp"
#include "game_engine/BoardController.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "game_engine/ClickPreparationService.hpp"
#include "game_engine/ScoreTracker.hpp"
#include "game_engine/MoveRecorder.hpp"
#include "game_engine/CooldownService.hpp"
#include "config/PieceSpeedConfig.hpp"

class GameEngine
{
public:
    explicit GameEngine(Board board, const PieceSpeedConfig& speedConfig);
    explicit GameEngine(Board board);

    void handleCellClick(int row, int col);
    void handleJump(int row, int col);
    void advanceTime(int milliseconds);
    GameSnapshot getSnapshot() const;
    GameState getState() const;
    const Board& getBoard() const;

private:
    Board board_;
    RuleEngine ruleEngine_;
    RealTimeArbiter arbiter_;
    BoardController boardController_;
    GameStateMachine stateMachine_;
    MoveCompletionService moveCompletionService_;
    ClickPreparationService clickPrepService_;
    const PieceSpeedConfig& speedConfig_;

    ScoreTracker scoreTracker_;
    MoveRecorder moveRecorder_;
    CooldownService cooldownService_;

    int gameTimeMs_ = 0;

    void executeDecision(const GameDecision& d);
    void handleGameOver(std::optional<Color> winner);
    void maybeReturnToSelection();

    int calculateMoveTime(int fromRow, int fromCol,
                          int toRow, int toCol, const Piece* piece) const;
};
