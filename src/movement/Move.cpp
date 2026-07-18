#include "movement/Move.hpp"
#include "model/Piece.hpp"
#include "model/Position.hpp"
#include <algorithm>
#include <cmath>

Move::Move(Piece* piece, Position from, Position to, int durationMs)
    : piece_(piece),
      from_(from),
      to_(to),
      remainingMs_(durationMs),
      durationMs_(durationMs),
      stoppedAt_(to),
      stopped_(false)
{
    buildPath();
}

void Move::buildPath()
{
    path_.clear();

    int dr = to_.row - from_.row;
    int dc = to_.col - from_.col;

    int steps = std::max(std::abs(dr), std::abs(dc));
    if (steps == 0)
    {
        path_.push_back(from_);
        return;
    }

    int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
    int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);

    for (int i = 0; i <= steps; ++i)
    {
        path_.push_back({from_.row + i * stepR, from_.col + i * stepC});
    }
}

void Move::stopAtCell(Position cell)
{
    stopped_ = true;
    stoppedAt_ = cell;
    // נחשב מחדש את הזמן שנותר לפי ההתקדמות למשבצת העצירה
    double progress = getProgressAtCell(cell);
    if (progress >= 0.0)
    {
        int elapsed = static_cast<int>(durationMs_ * progress);
        remainingMs_ = durationMs_ - elapsed;
        if (remainingMs_ < 0) remainingMs_ = 0;
    }
}

const std::vector<Position>& Move::getPath() const
{
    return path_;
}

Position Move::getCurrentCell() const
{
    if (stopped_)
        return stoppedAt_;

    double p = getProgress();
    if (p <= 0.0) return from_;
    if (p >= 1.0) return to_;

    int numSteps = static_cast<int>(path_.size()) - 1;
    if (numSteps <= 0) return from_;

    int stepIndex = static_cast<int>(p * numSteps);
    if (stepIndex < 0) stepIndex = 0;
    if (stepIndex > numSteps) stepIndex = numSteps;

    return path_[stepIndex];
}

double Move::getProgressAtCell(Position cell) const
{
    int numSteps = static_cast<int>(path_.size()) - 1;
    if (numSteps <= 0) return 0.0;

    for (size_t i = 0; i < path_.size(); ++i)
    {
        if (path_[i] == cell)
        {
            return static_cast<double>(i) / numSteps;
        }
    }
    return -1.0;  // לא נמצא במסלול
}

int Move::getMsPerStep() const
{
    int numSteps = static_cast<int>(path_.size()) - 1;
    if (numSteps <= 0) return durationMs_;
    return durationMs_ / numSteps;
}

// שאר המימוש ללא שינוי:
void Move::update(int milliseconds)
{
    remainingMs_ -= milliseconds;
    if (remainingMs_ < 0)
        remainingMs_ = 0;
}

bool Move::isFinished() const { return remainingMs_ <= 0; }
Piece* Move::getPiece() const { return piece_; }
Position Move::getFrom() const { return from_; }
Position Move::getTo() const { return stopped_ ? stoppedAt_ : to_; }
void Move::cancel() { cancelled_ = true; }
void Move::setIntercepted() { intercepted_ = true; }
bool Move::isIntercepted() const { return intercepted_; }
bool Move::isCancelled() const { return cancelled_; }

double Move::getProgress() const
{
    return 1.0 - static_cast<double>(remainingMs_) / durationMs_;
}
