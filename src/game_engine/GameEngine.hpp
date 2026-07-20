#pragma once
#include "game_engine/MoveCompletionService.hpp"
#include "model/Board.hpp"
#include "rule_engine/RuleEngine.hpp"
#include "arbiter/RealTimeArbiter.hpp"
#include "game_engine/GameStateMachine.hpp"
#include "game_engine/BoardController.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "game_engine/ClickPreparationService.hpp"
#include "config/PieceSpeedConfig.hpp"
#include <unordered_map>
#include <optional>

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
    int whiteScore_ = 0;
    int blackScore_ = 0;
    std::vector<MoveRecord> whiteMoves_;
    std::vector<MoveRecord> blackMoves_;
    int gameTimeMs_ = 0;  // accumulator for timestamps

    // ביצוע החלטות
    void executeDecision(const GameDecision& d);

    // עזרים ל-advanceTime
    void handleGameOver(std::optional<Color> winner);
    void recordMove(const CompletedMove& cm, bool isCapture);
    void recordJump(const JumpEntry& jump);
    static std::string formatMove(const MoveRecord& mr);
    void maybeReturnToSelection();

    // Pre-compute maps ל-getSnapshot
    std::unordered_map<const Piece*, double> buildMoveProgressMap() const;
    std::unordered_map<const Piece*, Position> buildMoveTargetMap() const;
    std::unordered_map<const Piece*, double> buildJumpProgressMap() const;

    // חישוב זמן מהלך
       int calculateMoveTime(int fromRow, int fromCol,
                          int toRow, int toCol, const Piece* piece) const;
        int getJumpDurationMs(PieceType type, Color color) const;


        // Cooldown tracker: Piece* → remainingRestMs
    std::unordered_map<Piece*, int> restTimers_;

    static constexpr int REST_AFTER_MOVE_MS = 2000;
    static constexpr int REST_AFTER_JUMP_MS = 500;
    const PieceSpeedConfig& speedConfig_;

    // עזרים:
    void startResting(Piece* piece, int durationMs);
    void advanceRestTimers(int milliseconds);
    bool isResting(Piece* piece) const;

};