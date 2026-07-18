#pragma once
#include "game_engine/MoveCompletionService.hpp"
#include "model/Board.hpp"
#include "rule_engine/RuleEngine.hpp"
#include "arbiter/RealTimeArbiter.hpp"
#include "game_engine/GameStateMachine.hpp"
#include "game_engine/BoardController.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "game_engine/ClickPreparationService.hpp"
#include <unordered_map>
#include <optional>

class GameEngine
{
public:
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
    static constexpr int JUMP_DURATION_MS = 1000;

    // ביצוע החלטות
    void executeDecision(const GameDecision& d);

    // עזרים ל-advanceTime
    void handleGameOver(std::optional<Color> winner);
    void maybeReturnToSelection();

    // Pre-compute maps ל-getSnapshot
    std::unordered_map<const Piece*, double> buildMoveProgressMap() const;
    std::unordered_map<const Piece*, Position> buildMoveTargetMap() const;
    std::unordered_map<const Piece*, double> buildJumpProgressMap() const;

    // חישוב זמן מהלך
    int calculateMoveTime(int fromRow, int fromCol,
                          int toRow, int toCol) const;

        // Cooldown tracker: Piece* → remainingRestMs
    std::unordered_map<const Piece*, int> restTimers_;

    static constexpr int REST_AFTER_MOVE_MS = 2000;
    static constexpr int REST_AFTER_JUMP_MS = 500;

    // עזרים:
    void startResting(Piece* piece, int durationMs);
    void advanceRestTimers(int milliseconds);
    bool isResting(const Piece* piece) const;

};