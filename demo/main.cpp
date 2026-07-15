/// @file demo/main.cpp
/// @brief פרויקט דמו GUI — תמונות אמיתיות + אנימציות Frame-based.

#include "ChessRenderer.hpp"
#include "DemoBoardState.hpp"
#include "DemoConfig.hpp"
#include "img.hpp"
#include <iostream>
#include <string>
#include <filesystem>

int main(int argc, char* argv[])
{
    namespace fs = std::filesystem;

    // ── 1. תיקיית assets ──
    std::string assetsPath = "assets";
    if (argc >= 2)
        assetsPath = argv[1];

    if (!fs::exists(assetsPath))
    {
        std::string altPath = fs::path(__FILE__).parent_path().string() + "/assets";
        if (fs::exists(altPath))
            assetsPath = altPath;
    }

    std::cout << "Assets path: " << assetsPath << '\n';

    // ── 2. אתחול Renderer ──
    ChessRenderer renderer;
    if (!renderer.initialize(assetsPath))
    {
        std::cerr << "Failed to initialize renderer.\n";
        return 1;
    }

    // ── 3. Game state (דמו) ──
    DemoBoardState gameState;

    // ── 4. חיבור עכבר ← לוגיקה ──
    renderer.setClickCallback([&gameState, &renderer](int row, int col) {
        // איפוס אנימציה של כלי שמתחיל לזוז
        gameState.handleClick(row, col);

        // אם מתחיל מהלך — מאפס animation elapsed
        GameSnapshot snap = gameState.getSnapshot();
        for (const auto& p : snap.pieces)
        {
            if (p.state == PieceState::Moving && p.progress == 0.0)
                renderer.getAnimMgr().reset(p.pieceId);
        }
    });
    renderer.attachMouse();

    // ── 5. לולאת משחק ──
    bool running = true;
    int  frameCount = 0;

    std::cout << "Starting game loop... (ESC or 'q' to quit)\n";

    while (running)
    {
        gameState.advanceTime(DemoConfig::FRAME_DELAY_MS);

        GameSnapshot snapshot = gameState.getSnapshot();

        renderer.render(snapshot);
        renderer.display();

        int key = Img::wait_key(DemoConfig::FRAME_DELAY_MS);
        if (key == 27 || key == 'q' || key == 'Q')
            running = false;

        if (!renderer.isWindowOpen())
            running = false;

        ++frameCount;
    }

    double elapsed = frameCount * DemoConfig::FRAME_DELAY_MS / 1000.0;
    std::cout << "Exited. " << frameCount << " frames in ~"
              << elapsed << " seconds.\n";

    Img::destroy_all_windows();
    return 0;
}
