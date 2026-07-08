#include <gtest/gtest.h>
#include "Game.hpp"
#include "Board.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Knight.hpp"
#include "Queen.hpp"
#include "Pawn.hpp"

TEST(GameTest, PieceArrivesAfterEnoughTime)
{
    Board board({
        {"wR",".","."}
    });

    Game game(board);

    game.click(50,50);
    game.click(250,50);

    game.wait(2000);

    EXPECT_EQ(game.getBoard().getCell(0,0), nullptr);
    EXPECT_NE(game.getBoard().getCell(0,2), nullptr);
}
TEST(GameTest, MovingPieceCannotBeRedirected)
{
    Board board({
        {"wR",".","."}
    });

    Game game(board);

    game.click(50,50);
    game.click(250,50);

    game.wait(1000);

    game.click(50,50);
    game.click(150,50);

    game.wait(1000);

    EXPECT_NE(game.getBoard().getCell(0,2), nullptr);
}
TEST(GameTest, PieceCanMoveAgainImmediately)
{
    Board board({
        {"wR",".","."}
    });

    Game game(board);

    game.click(50,50);
    game.click(250,50);

    game.wait(2000);

    game.click(250,50);
    game.click(150,50);

    game.wait(1000);

    EXPECT_NE(game.getBoard().getCell(0,1), nullptr);
}
TEST(GameTest, SameColorMovesTogether)
{
    Board board({
        {"wR",".","wR"}
    });

    Game game(board);

    game.click(50,50);
    game.click(150,50);

    game.click(250,50);
    game.click(150,50);

    game.wait(1000);

    EXPECT_NE(game.getBoard().getCell(0,1), nullptr);
}
TEST(GameTest, WhiteWinsCollision)
{
    Board board({
        {"wR",".",".","bR"}
    });

    Game game(board);

    game.click(50,50);
    game.click(350,50);

    game.click(350,50);
    game.click(50,50);

    game.wait(3000);

    EXPECT_NE(game.getBoard().getCell(0,3), nullptr);
    EXPECT_EQ(game.getBoard().getCell(0,0), nullptr);
}