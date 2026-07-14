#pragma once

#include "img.hpp"
#include <map>
#include <string>
#include <vector>

/// מייצג כלי בודד על כל ה-states והאנימציות שלו.
/// טוען תיקייה כמו "assets/pieces2/KW" — קורא config.json + frames.
class AnimatedSprite
{
public:
    using StateFrames = std::vector<Img>;

    struct Config
    {
        double fps    = 6.0;
        bool   loop   = true;
        double speedMps = 0.0;
        std::string nextState;
    };

    /// טוען תיקיית כלי (e.g. "assets/pieces2/KW")
    /// התיקייה צריכה להכיל תת-תיקיות לכל state: idle, move, jump, ...
    /// כל state מכיל config.json + sprites/1.png, 2.png, ...
    bool load(const std::string& pieceFolder);

    /// ממיר PieceState לשם תיקיית state ("idle", "move", "jump")
    static std::string pieceStateToFolder(int pieceState);

    /// מחזיר את הפריים הנוכחי לפי elapsed (בשניות)
    const Img& getFrame(const std::string& stateName, int frameIndex) const;

    /// מחזיר את כמות הפריימים ב-state
    int getFrameCount(const std::string& stateName) const;

    /// הגישה לקונפיג של state
    const Config& getConfig(const std::string& stateName) const;

    /// בדיקה האם ה-state קיים
    bool hasState(const std::string& stateName) const;

    /// רשימת כל ה-states
    const std::map<std::string, StateFrames>& getAllStates() const { return states_; }

private:
    std::map<std::string, StateFrames> states_;
    std::map<std::string, Config>      configs_;

    bool parseConfig(const std::string& jsonPath, Config& cfg);
};
