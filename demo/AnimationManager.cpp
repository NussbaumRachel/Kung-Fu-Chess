#include "AnimationManager.hpp"

void AnimationManager::update(int pieceId, int pieceState, double deltaSec)
{
    auto& st = states_[pieceId];

    std::string stateName = AnimatedSprite::pieceStateToFolder(pieceState); // "idle"/"move"/"jump"

    // state changed → reset
    if (st.currentState != stateName)
    {
        st.currentState = stateName;
        st.elapsed      = 0.0;
        st.currentFrame = 0;
    }

    st.elapsed += deltaSec;

    // frameIndex לא מחושב פה — מחושב ב-PieceRenderer לפי fps של ה-AnimatedSprite
    // אבל נאגור frameIndex פשוט לנוחות
    // — frameIndex יחושב ב-PieceRenderer ישירות
    st.currentFrame = static_cast<int>(st.elapsed * 12.0); // default 12fps, יוחלף ע"י fps מהקונפיג
}

void AnimationManager::reset(int pieceId)
{
    auto& st = states_[pieceId];
    st.elapsed      = 0.0;
    st.currentFrame = 0;
}

int AnimationManager::getFrameIndex(int pieceId) const
{
    auto it = states_.find(pieceId);
    if (it != states_.end()) return it->second.currentFrame;
    return 0;
}

const std::string& AnimationManager::getCurrentStateName(int pieceId) const
{
    static std::string empty;
    auto it = states_.find(pieceId);
    if (it != states_.end()) return it->second.currentState;
    return empty;
}

double AnimationManager::getElapsed(int pieceId) const
{
    auto it = states_.find(pieceId);
    if (it != states_.end()) return it->second.elapsed;
    return 0.0;
}
