#include "GameServer.hpp"
#include "controllerClick/GameController.hpp"
#include "game_engine/GameEngine.hpp"
#include "model/Board.hpp"
#include "config/PieceSpeedConfig.hpp"

#include <iostream>
#include <string>
#include <filesystem>


int main(int argc, char* argv[])
{
    try
    {
        // Assets path
        std::string assetsPath = "assets";
        if (argc >= 2)
            assetsPath = argv[1];
        if (!std::filesystem::exists(assetsPath))
        {
            std::string altPath =
                std::filesystem::path(__FILE__).parent_path().string()
                + "/assets";
            if (std::filesystem::exists(altPath))
                assetsPath = altPath;
        }

        // Board
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

        // Speed config
        PieceSpeedConfig speedCfg;
        speedCfg.load(assetsPath + "/pieces");

        // Engine + Controller (same pattern as demo/main.cpp)
        GameEngine engine(std::move(board), speedCfg);
        GameController controller(engine);

        // Server
        GameServer server(8080, controller);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
