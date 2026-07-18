#ifndef MOVE_HPP
#define MOVE_HPP

#include "model/Position.hpp"
#include <vector>

class Piece;

class Move
{
public:
    Move(Piece* piece, Position from, Position to, int durationMs);

    void update(int milliseconds);

    bool isFinished() const;
    bool isCancelled() const;
    void cancel();
    void setIntercepted();
    bool isIntercepted() const;
    double getProgress() const;

    Piece* getPiece() const;
    Position getFrom() const;
    Position getTo() const;
    int getDurationMs() const { return durationMs_; }
    int getRemainingMs() const { return remainingMs_; }

    // ── חדש: מסלול ──
    // מחזיר את רשימת המשבצות שהכלי עובר דרכן (כולל from, לא כולל to בתור צעד אחרון?)
    // בפועל: [from, ..., to], כלומר from=index 0, to=index last
    const std::vector<Position>& getPath() const;

    // באיזו משבצת הכלי נמצא כרגע (לפי progress)?
    Position getCurrentCell() const;

    // progress שבו הכלי מגיע למשבצת מסוימת במסלול (0.0=from, 1.0=to)
    // מחזיר -1.0 אם המשבצת לא במסלול
    double getProgressAtCell(Position cell) const;

    // משך זמן לכל משבצת במסלול (ms)
    int getMsPerStep() const;

    // עצירת מהלך במשבצת מסוימת (במקום להגיע ל-to)
    void stopAtCell(Position cell);

private:
    Piece* piece_;
    Position from_;
    Position to_;
    bool cancelled_ = false;
    bool intercepted_ = false;
    int remainingMs_;
    int durationMs_;

    // ── חדש: מסלול מחושב בבנייה ──
    std::vector<Position> path_;
    Position stoppedAt_;           // אם נעצר — באיזו משבצת (ברירת מחדל = to)
    bool stopped_ = false;

    void buildPath();
};

#endif
