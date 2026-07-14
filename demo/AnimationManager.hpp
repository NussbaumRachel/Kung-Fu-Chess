#pragma once

#include "img.hpp"
#include "AnimatedSprite.hpp"
#include <unordered_map>
#include <string>

/// מנהל מצב אנימציה עבור כל כלי פעיל: איזה frame, elapsed time.
class AnimationManager
{
public:
    /// עדכון elapsed time עבור כלי
    void update(int pieceId, int pieceState, double deltaSec);

    /// איפוס מצב כלי (למשל, התחלת move)
    void reset(int pieceId);

    /// מחזיר frameIndex נוכחי
    int getFrameIndex(int pieceId) const;

    /// מחזיר את שם ה-state שהיה בשימוש
    const std::string& getCurrentStateName(int pieceId) const;

    /// מחזיר elapsed בשניות
    double getElapsed(int pieceId) const;

private:
    struct AnimState
    {
        double      elapsed      = 0.0;
        int         currentFrame = 0;
        std::string currentState;
    };

    std::unordered_map<int, AnimState> states_;
};
