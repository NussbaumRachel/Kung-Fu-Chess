#include "ChessRenderer.hpp"
#include "DemoConfig.hpp"
#include "game_engine/GameEngine.hpp"
#include "controllerClick/GameController.hpp"
#include "text_io/BoardParser.hpp"
#include "img.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <sstream>

int main(int argc, char* argv[])
{
    namespace fs = std::filesystem;
    
    // ── 1. Assets path ──
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
    
    // ── 2. Initialize Renderer ──
    ChessRenderer renderer;
    if (!renderer.initialize(assetsPath))
    {
        std::cerr << "Failed to initialize renderer.\n";
        return 1;
    }
    
    // ── 3. Create Board ──
    Board board({
        {"bR","bN","bB","bQ","bK","bB","bN","bR"},
        {"bP","bP","bP","bP","bP","bP","bP","bP"},
        {".",".",".",".",".",".",".","."},
        {".",".",".",".",".",".",".","."},
        {".",".",".",".",".",".",".","."},
        {".",".",".",".",".",".",".","."},
        {"wP","wP","wP","wP","wP","wP","wP","wP"},
        {"wR","wN","wB","wQ","wK","wB","wN","wR"}
    });
    
    // ── 4. Engine + Controller ──
    GameEngine engine(std::move(board));
    GameController controller(engine);

    // ── 5. Mouse callback ──
    renderer.setClickCallback([&controller, &renderer](int row, int col) {
        controller.handleCellClick(row, col);

        // איפוס AnimationManager לכל Piece שהתחיל לזוז
        GameSnapshot snap = controller.getSnapshot();
        for (const auto& p : snap.pieces)
        {
            if (p.state == PieceState::Moving && p.progress == 0.0)
                renderer.getAnimMgr().reset(p.pieceId);
            else if (p.state == PieceState::Jumping && p.progress == 0.0)
                renderer.getAnimMgr().reset(p.pieceId);
        }
    });
    renderer.attachMouse();
    
    // ── 6. Main loop ──
    bool running = true;
    while (running)
    {
        engine.advanceTime(DemoConfig::FRAME_DELAY_MS);
        
        GameSnapshot snapshot = engine.getSnapshot();
        renderer.render(snapshot);
        renderer.display();
        
        int key = Img::wait_key(DemoConfig::FRAME_DELAY_MS);
        if (key == 27) // ESC
            running = false;
        
        if (!renderer.isWindowOpen())
            running = false;
    }
    
    Img::destroy_all_windows();
    return 0;
}
