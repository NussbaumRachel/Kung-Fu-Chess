#pragma once

#include "img.hpp"
#include <memory>
#include <string>
#include "DemoConfig.hpp"
#include "GameSnapshot.hpp"
#include "AssetManager.hpp"
#include "AnimationManager.hpp"
#include "BoardRenderer.hpp"
#include "PieceRenderer.hpp"
#include "MouseHandler.hpp"

/// מתזמר הצגה. AssetManager + AnimationManager.
class ChessRenderer
{
public:
    ChessRenderer();

    bool initialize(const std::string& assetsPath);

    void setClickCallback(MouseHandler::ClickCallback callback);
    void attachMouse();

    void render(const GameSnapshot& snapshot);
    void display();
    bool isWindowOpen() const;

    AnimationManager& getAnimMgr() { return animMgr_; }
    const AssetManager& getAssets() const { return *assets_; }

private:
    std::unique_ptr<AssetManager>   assets_;
    AnimationManager                animMgr_;
    BoardRenderer                   boardRenderer_;
    PieceRenderer                   pieceRenderer_;
    std::unique_ptr<MouseHandler>   mouseHandler_;

    Img         canvas_;
    std::string windowName_;

    void drawGameOverlay(const GameSnapshot& snapshot);
};
