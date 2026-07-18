#include "arbiter/RealTimeArbiter.hpp"
#include "model/Piece.hpp"

// ──────────────── Moves ────────────────

void RealTimeArbiter::startMove(Piece* piece,
                                Position from, Position to,
                                int durationMs)
{
    activeMoves_.emplace_back(piece, from, to, durationMs);
}

// ──────────────── Jumps ────────────────

void RealTimeArbiter::startJump(Piece* piece, Position cell, int durationMs)
{
    activeJumps_.push_back({piece, cell, durationMs});
}

// ──────────────── advanceTime ────────────────

void RealTimeArbiter::advanceTime(int milliseconds, const Board& board)
{
    completedMoves_.clear();
    completedJumps_.clear();

    resolveCollisions();

    // ── יירוטים: קופץ לוכד אויב שמגיע לתא שלו ──
    // חייב לרוץ לפני שה-jumps מתעדכנים, כדי שקפיצות שעדיין פעילות
    // (גם אם הן עומדות להסתיים באותו tick) יוכלו ליירט מהלכים.
    resolveJumpInterceptions();
    resolvePathCollisions(board);       // שלב 3: התנגשות במסלול (חדש)

    // ── עדכון Moves ──
    for (auto it = activeMoves_.begin(); it != activeMoves_.end(); )
    {
        it->update(milliseconds);

        if (it->isCancelled())
        {
            CompletedMove cm;
            cm.from = it->getFrom();
            cm.to = it->getTo();
            cm.piece = it->getPiece();
            cm.wasCancelled = true;
            cm.wasIntercepted = it->isIntercepted();
            completedMoves_.push_back(cm);
            it = activeMoves_.erase(it);
        }
        else if (it->isFinished())
        {
            CompletedMove cm;
            cm.from = it->getFrom();
            cm.to = it->getTo();
            cm.piece = it->getPiece();
            cm.wasCancelled = false;
            completedMoves_.push_back(cm);
            it = activeMoves_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // ── עדכון Jumps ──
    for (auto it = activeJumps_.begin(); it != activeJumps_.end(); )
    {
        it->remainingMs -= milliseconds;
        if (it->remainingMs <= 0)
        {
            it->remainingMs = 0;
            completedJumps_.push_back(*it);
            it = activeJumps_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// ──────────────── Poll ────────────────

std::vector<CompletedMove> RealTimeArbiter::pollCompletedMoves()
{
    return std::move(completedMoves_);
}

std::vector<JumpEntry> RealTimeArbiter::pollCompletedJumps()
{
    return std::move(completedJumps_);
}

// ──────────────── Queries ────────────────

bool RealTimeArbiter::hasActiveMoves() const
{
    return !activeMoves_.empty();
}

bool RealTimeArbiter::hasActiveJumps() const
{
    return !activeJumps_.empty();
}

bool RealTimeArbiter::isCellInvolved(int row, int col) const
{
    // בדיקת moves (from/to)
    for (const Move& move : activeMoves_)
    {
        Position from = move.getFrom();
        Position to = move.getTo();
        if ((from.row == row && from.col == col) ||
            (to.row == row && to.col == col))
        {
            return true;
        }
    }
    // בדיקת jumps
    for (const JumpEntry& jump : activeJumps_)
    {
        if (jump.cell.row == row && jump.cell.col == col)
        {
            return true;
        }
    }
    return false;
}

bool RealTimeArbiter::isPieceInvolved(int row, int col) const
{
    // בודק רק מקורות (from) — לא targets.
    // כלי בתא (row,col) מעורב רק אם הוא יצא לדרך
    for (const Move& move : activeMoves_)
    {
        Position from = move.getFrom();
        if (from.row == row && from.col == col)
            return true;
    }
    // jump — תמיד source = target cell, אבל גם כאן בודקים
    for (const JumpEntry& jump : activeJumps_)
    {
        if (jump.cell.row == row && jump.cell.col == col)
            return true;
    }
    return false;
}

const std::vector<Move>& RealTimeArbiter::getActiveMoves() const
{
    return activeMoves_;
}

const std::vector<JumpEntry>& RealTimeArbiter::getActiveJumps() const
{
    return activeJumps_;
}

// ──────────────── Collisions ────────────────

void RealTimeArbiter::resolveCollisions()
{
    for (size_t i = 0; i < activeMoves_.size(); ++i)
    {
        for (size_t j = i + 1; j < activeMoves_.size(); ++j)
        {
            Move& first = activeMoves_[i];
            Move& second = activeMoves_[j];

            if (first.isCancelled() || second.isCancelled())
                continue;

            Piece* firstPiece = first.getPiece();
            Piece* secondPiece = second.getPiece();

            if (firstPiece == nullptr || secondPiece == nullptr)
                continue;

            // התנגשות בין כלים מיריבים באותו יעד
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

// ──────────────── Jump Interceptions ────────────────

void RealTimeArbiter::resolveJumpInterceptions()
{
    for (auto& jump : activeJumps_)
    {
        for (auto& move : activeMoves_)
        {
            if (move.isCancelled() || move.isFinished())
                continue;

            // בודק האם אויב בתנועה מגיע לתא של הקופץ
            if (move.getTo() == jump.cell &&
                move.getPiece() != nullptr &&
                move.getPiece()->getColor() != jump.piece->getColor())
            {
                // הקופץ לוכד את האויב — האויב מבוטל
                move.cancel();
                move.setIntercepted();
            }
        }
    }
}
void RealTimeArbiter::resolvePathCollisions(const Board& board)
{
    // ── התנגשות בין שני מהלכים פעילים ──
    for (size_t i = 0; i < activeMoves_.size(); ++i)
    {
        for (size_t j = i + 1; j < activeMoves_.size(); ++j)
        {
            Move& move1 = activeMoves_[i];
            Move& move2 = activeMoves_[j];

            if (move1.isCancelled() || move2.isCancelled())
                continue;
            if (move1.isFinished() || move2.isFinished())
                continue;

            Piece* p1 = move1.getPiece();
            Piece* p2 = move2.getPiece();
            if (!p1 || !p2) continue;

            bool sameColor = (p1->getColor() == p2->getColor());

            const auto& path1 = move1.getPath();
            const auto& path2 = move2.getPath();

            // מצא חיתוך בין המסלולים
            for (const Position& cell : path1)
            {
                double progress1 = move1.getProgressAtCell(cell);
                double progress2 = move2.getProgressAtCell(cell);

                if (progress1 < 0.0 || progress2 < 0.0)
                    continue;  // לא משותף

                // חשב זמן הגעה (ms) לכל משבצת
                int time1 = static_cast<int>(progress1 * move1.getDurationMs());
                int time2 = static_cast<int>(progress2 * move2.getDurationMs());

                // חפיפה בזמן?
                int diff = std::abs(time1 - time2);
                int threshold = std::max(move1.getMsPerStep(), move2.getMsPerStep()) / 2;

                if (diff <= threshold)
                {
                    if (sameColor)
                    {
                        // ידידותי — מי שמגיע מאוחר יותר נעצר
                        if (time1 > time2)
                        {
                            // move1 מגיע מאוחר — נעצר במשבצת הקודמת
                            int idx = static_cast<int>(progress1 * (path1.size() - 1));
                            if (idx > 0)
                                move1.stopAtCell(path1[idx - 1]);
                            else
                                move1.cancel();  // אין משבצת קודמת — מבטלים
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
                        // אויב — מי שמגיע ראשון אוכל את השני
                        if (time1 <= time2)
                            move2.cancel();   // move1 אוכל את move2
                        else
                            move1.cancel();   // move2 אוכל את move1
                    }
                }
            }
        }
    }

    // ── התנגשות בין מהלך לכלי נייח (Idle/Resting) על הלוח ──
    for (Move& move : activeMoves_)
    {
        if (move.isCancelled() || move.isFinished())
            continue;

        Piece* movingPiece = move.getPiece();
        if (!movingPiece) continue;

        const auto& path = move.getPath();

                for (size_t idx = 1; idx < path.size(); ++idx)
        {
            const Position& cell = path[idx];

            if (board.isEmptyCell(cell.row, cell.col))
                continue;

            Piece* stationaryPiece = board.getCell(cell.row, cell.col);
            if (!stationaryPiece) continue;
            if (stationaryPiece == movingPiece) continue;

            bool sameColor = (stationaryPiece->getColor() == movingPiece->getColor());

            if (sameColor)
            {
                // חבר חוסם — עצור במשבצת הקודמת
                int prevIdx = static_cast<int>(idx) - 1;
                if (prevIdx > 0)
                    move.stopAtCell(path[prevIdx]);
                else
                    move.cancel();
            }
            else
            {
                // אויב נייח — אוכלים אותו, ממשיכים
                // האויב יוסר מה-board ב-completeMove (כבר מטופל)
                // לא צריך לעשות כלום כאן — moveCompletionService יטפל
            }
        }

    }
}
