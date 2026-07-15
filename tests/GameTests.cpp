#include <gtest/gtest.h>
#include "game_engine/GameEngine.hpp"
#include "model/Board.hpp"
#include "model/Rook.hpp"
#include "model/Bishop.hpp"
#include "model/Knight.hpp"
#include "model/Queen.hpp"
#include "model/Pawn.hpp"

TEST(GameTest, PieceArrivesAfterEnoughTime)
{
    Board board({
        {"wR",".","."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 2);

    engine.advanceTime(2000);

    EXPECT_EQ(engine.getBoard().getCell(0, 0), nullptr);
    EXPECT_NE(engine.getBoard().getCell(0, 2), nullptr);
}

TEST(GameTest, MovingPieceCannotBeRedirected)
{
    Board board({
        {"wR",".","."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 2);

    engine.advanceTime(1000);

    // ניסיון להפנות — אמור להיכשל, הכלי בתנועה
    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 1);

    engine.advanceTime(1000);

    EXPECT_NE(engine.getBoard().getCell(0, 2), nullptr);
}

TEST(GameTest, PieceCanMoveAgainImmediately)
{
    Board board({
        {"wR",".","."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 2);

    engine.advanceTime(2000);

    engine.handleCellClick(0, 2);
    engine.handleCellClick(0, 1);

    engine.advanceTime(1000);

    EXPECT_NE(engine.getBoard().getCell(0, 1), nullptr);
}

TEST(GameTest, SameColorMovesTogether)
{
    Board board({
        {"wR",".","wR"}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 1);

    engine.handleCellClick(0, 2);
    engine.handleCellClick(0, 1);

    engine.advanceTime(1000);

    EXPECT_NE(engine.getBoard().getCell(0, 1), nullptr);
}

TEST(GameTest, WhiteWinsCollision)
{
    Board board({
        {"wR",".",".","bR"}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 3);

    engine.handleCellClick(0, 3);
    engine.handleCellClick(0, 0);

    engine.advanceTime(3000);

    EXPECT_NE(engine.getBoard().getCell(0, 3), nullptr);
    EXPECT_EQ(engine.getBoard().getCell(0, 0), nullptr);
}
