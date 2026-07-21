#include "arbiter/CollisionResolver.hpp"
#include "arbiter/RealTimeArbiter.hpp"   // for full JumpEntry definition
#include "model/Board.hpp"
#include "model/Piece.hpp"
#include <algorithm>
#include <cmath>

void CollisionResolver::resolve(
    std::vector<Move>& activeMoves,
    std::vector<JumpEntry>& activeJumps,
    Board& board)
{
    resolveTargetCollisions(activeMoves);
    resolveJumpInterceptions(activeMoves, activeJumps);
    resolvePathCollisions(activeMoves, board);
}

// ── Phase 1: Two enemy pieces targeting same destination ──
void CollisionResolver::resolveTargetCollisions(
    std::vector<Move>& activeMoves)
{
    for (size_t i = 0; i < activeMoves.size(); ++i)
    {
        for (size_t j = i + 1; j < activeMoves.size(); ++j)
        {
            Move& first = activeMoves[i];
            Move& second = activeMoves[j];

            if (first.isCancelled() || second.isCancelled())
                continue;

            Piece* firstPiece = first.getPiece();
            Piece* secondPiece = second.getPiece();

            if (firstPiece == nullptr || secondPiece == nullptr)
                continue;

            if (firstPiece->getColor() != secondPiece->getColor())
            {
                if (first.getTo() == second.getTo())
                {
                    if (first.getRemainingMs() <= second.getRemainingMs())
                        second.cancel();
                    else
                        first.cancel();
                }
            }
        }
    }
}

// ── Phase 2: Jumping piece intercepts enemy moving into its cell ──
void CollisionResolver::resolveJumpInterceptions(
    std::vector<Move>& activeMoves,
    std::vector<JumpEntry>& activeJumps)
{
    for (auto& jump : activeJumps)
    {
        for (auto& move : activeMoves)
        {
            if (move.isCancelled() || move.isFinished())
                continue;

            if (move.getTo() == jump.cell &&
                move.getPiece() != nullptr &&
                move.getPiece()->getColor() != jump.piece->getColor())
            {
                move.cancel();
                move.setIntercepted();
            }
        }
    }
}

// ── Phase 3a: Move-vs-Move along paths ──
static void resolveMoveVsMove(std::vector<Move>& activeMoves, size_t i, size_t j)
{
    Move& move1 = activeMoves[i];
    Move& move2 = activeMoves[j];

    if (move1.isCancelled() || move2.isCancelled())
        return;
    if (move1.isFinished() || move2.isFinished())
        return;

    Piece* p1 = move1.getPiece();
    Piece* p2 = move2.getPiece();
    if (!p1 || !p2) return;

    bool sameColor = (p1->getColor() == p2->getColor());

    const auto& path1 = move1.getPath();
    const auto& path2 = move2.getPath();

    int actualElapsed1 = move1.getDurationMs() - move1.getRemainingMs();
    int actualElapsed2 = move2.getDurationMs() - move2.getRemainingMs();

    for (const Position& cell : path1)
    {
        double progress1 = move1.getProgressAtCell(cell);
        double progress2 = move2.getProgressAtCell(cell);

        if (progress1 < 0.0 || progress2 < 0.0)
            continue;

        int elapsedToCell1 = static_cast<int>(progress1 * move1.getDurationMs());
        int elapsedToCell2 = static_cast<int>(progress2 * move2.getDurationMs());
        int remaining1 = elapsedToCell1 - actualElapsed1;
        int remaining2 = elapsedToCell2 - actualElapsed2;
        if (remaining1 < 0) remaining1 = 0;
        if (remaining2 < 0) remaining2 = 0;

        int diff = std::abs(remaining1 - remaining2);
        int threshold = std::max(move1.getMsPerStep(), move2.getMsPerStep()) / 2;

        if (diff <= threshold)
        {
            if (sameColor)
            {
                if (remaining1 > remaining2)
                {
                    int idx = static_cast<int>(progress1 * (path1.size() - 1));
                    if (idx > 0)
                        move1.stopAtCell(path1[idx - 1]);
                    else
                        move1.cancel();
                }
                else
                {
                    int idx = static_cast<int>(progress2 * (path2.size() - 1));
                    if (idx > 0)
                        move2.stopAtCell(path2[idx - 1]);
                    else
                        move2.cancel();
                }
            }
            else
            {
                if (remaining1 <= remaining2)
                {
                    move2.cancel();
                    move2.setIntercepted();
                }
                else
                {
                    move1.cancel();
                    move1.setIntercepted();
                }
            }
            break;
        }
    }
}

// ── Phase 3b: Move vs stationary piece on the board ──
static void resolveMoveVsStationary(std::vector<Move>& activeMoves, Board& board)
{
    for (Move& move : activeMoves)
    {
        if (move.isCancelled() || move.isFinished())
            continue;

        Piece* movingPiece = move.getPiece();
        if (!movingPiece) continue;

        Position currentCell = move.getCurrentCell();

        if (currentCell == move.getFrom())
            continue;

        if (board.isEmptyCell(currentCell.row, currentCell.col))
            continue;

        Piece* otherPiece = board.getCell(currentCell.row, currentCell.col);
        if (!otherPiece) continue;
        if (otherPiece == movingPiece) continue;

        bool sameColor = (otherPiece->getColor() == movingPiece->getColor());

        if (sameColor)
        {
            const auto& path = move.getPath();
            for (size_t i = 0; i < path.size(); ++i)
            {
                if (path[i] == currentCell && i > 0)
                {
                    move.stopAtCell(path[i - 1]);
                    break;
                }
            }
        }
        else
        {
            otherPiece->setState(PieceState::Captured);
            board.takeCell(currentCell.row, currentCell.col);
        }
    }
}

void CollisionResolver::resolvePathCollisions(
    std::vector<Move>& activeMoves,
    Board& board)
{
    // Phase 3a: move-vs-move path collisions
    for (size_t i = 0; i < activeMoves.size(); ++i)
    {
        for (size_t j = i + 1; j < activeMoves.size(); ++j)
        {
            resolveMoveVsMove(activeMoves, i, j);
        }
    }

    // Phase 3b: move-vs-stationary piece
    resolveMoveVsStationary(activeMoves, board);
}
