#include "AnimatedSprite.hpp"
#include "config/PieceConfigReader.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// ════════════════════════════════════════════════
//  pieceStateToFolder
// ════════════════════════════════════════════════

std::string AnimatedSprite::pieceStateToFolder(int pieceState)
{
    // PieceState enum: Idle=0, Moving=1, Jumping=2
    switch (pieceState)
    {
        case 0: return "idle";
        case 1: return "move";
        case 2: return "jump";
        case 3: return "long_rest";      // חדש
        default: return "idle";
    }
}

// ════════════════════════════════════════════════
//  parseConfig — פירוק JSON פשוט (בלי ספריית JSON)
// ════════════════════════════════════════════════

bool AnimatedSprite::parseConfig(const std::string& jsonPath, Config& cfg)
{
    std::ifstream file(jsonPath);
    if (!file.is_open()) return false;
    file.close();  // PieceConfigReader יפתח מחדש

    cfg.fps      = PieceConfigReader::readDouble(jsonPath, "frames_per_sec", 6.0);
    cfg.loop     = PieceConfigReader::readBool(jsonPath, "is_loop", true);
    cfg.speedMps = PieceConfigReader::readDouble(jsonPath, "speed_m_per_sec", 0.0);
    cfg.nextState = PieceConfigReader::readString(jsonPath, "next_state_when_finished", "");

    return true;
}


// ════════════════════════════════════════════════
//  load — טעינת תיקיית כלי
// ════════════════════════════════════════════════

bool AnimatedSprite::load(const std::string& pieceFolder)
{
    if (!fs::exists(pieceFolder) || !fs::is_directory(pieceFolder))
    {
        std::cerr << "[AnimatedSprite] Folder not found: " << pieceFolder << '\n';
        return false;
    }

    // מצפה: pieceFolder/states/<stateName>/sprites/*.png + config.json
    std::string statesPath = pieceFolder + "/states";
    if (!fs::exists(statesPath))
    {
        std::cerr << "[AnimatedSprite] No 'states' in: " << pieceFolder << '\n';
        return false;
    }

    for (const auto& stateEntry : fs::directory_iterator(statesPath))
    {
        if (!stateEntry.is_directory()) continue;

        std::string stateName = stateEntry.path().filename().string();
        std::string statePath = stateEntry.path().string();

        // config.json
        std::string configPath = statePath + "/config.json";
        Config cfg;
        if (fs::exists(configPath))
            parseConfig(configPath, cfg);
        configs_[stateName] = cfg;

        // sprites/*.png
        std::string spritesPath = statePath + "/sprites";
        StateFrames frames;

        if (fs::exists(spritesPath))
        {
            // איסוף קבצי PNG לפי סדר מספרי
            std::vector<std::string> pngFiles;
            for (const auto& f : fs::directory_iterator(spritesPath))
            {
                if (f.path().extension() == ".png")
                    pngFiles.push_back(f.path().string());
            }
            // מיון לפי מספר
            std::sort(pngFiles.begin(), pngFiles.end());

            for (const auto& png : pngFiles)
            {
                try
                {
                    Img frame;
                    frame.read(png);
                    frames.push_back(std::move(frame));
                }
                catch (const std::exception& e)
                {
                    std::cerr << "[AnimatedSprite] Cannot load: " << png
                              << " (" << e.what() << ")\n";
                }
            }
        }

        states_[stateName] = std::move(frames);
    }

    std::cout << "[AnimatedSprite] Loaded: " << pieceFolder
              << " (" << states_.size() << " states)\n";
    return !states_.empty();
}

// ════════════════════════════════════════════════
//  גישה
// ════════════════════════════════════════════════

const Img& AnimatedSprite::getFrame(const std::string& stateName, int frameIndex) const
{
    auto it = states_.find(stateName);
    if (it == states_.end() || it->second.empty())
    {
        static Img empty;
        return empty;
    }

    int idx = frameIndex % static_cast<int>(it->second.size());
    return it->second[idx];
}

int AnimatedSprite::getFrameCount(const std::string& stateName) const
{
    auto it = states_.find(stateName);
    if (it == states_.end()) return 0;
    return static_cast<int>(it->second.size());
}

const AnimatedSprite::Config& AnimatedSprite::getConfig(const std::string& stateName) const
{
    static Config defaultCfg;
    auto it = configs_.find(stateName);
    if (it != configs_.end()) return it->second;
    return defaultCfg;
}

bool AnimatedSprite::hasState(const std::string& stateName) const
{
    auto it = states_.find(stateName);
    return it != states_.end() && !it->second.empty();
}
