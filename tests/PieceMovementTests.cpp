#include <gtest/gtest.h>

#include "Board.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Knight.hpp"
#include "Queen.hpp"
#include "Pawn.hpp"

TEST(RookTest, CannotMoveThroughPiece)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wR", "."},
        {"wP", "."},
        {".", "."}
    };

    Board board(layout);

    Piece* rook = board.getCell(0, 0);

    EXPECT_FALSE(
        rook->isValidMove(0, 0, 2, 0, board)
    );
}


TEST(BishopTest, CannotMoveThroughPiece)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wB", ".", "."},
        {".", "wP", "."},
        {".", ".", "."}
    };

    Board board(layout);

    Piece* bishop = board.getCell(0, 0);

    EXPECT_FALSE(
        bishop->isValidMove(0, 0, 2, 2, board)
    );
}


TEST(KnightTest, CanJumpOverPieces)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wN", "wP", "."},
        {"wP", "wP", "."},
        {".", ".", "."}
    };

    Board board(layout);

    Piece* knight = board.getCell(0, 0);

    EXPECT_TRUE(
        knight->isValidMove(0, 0, 2, 1, board)
    );
}


TEST(RookTest, CannotCaptureFriendlyPiece)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wR", "."},
        {"wP", "."}
    };

    Board board(layout);

    Piece* rook = board.getCell(0, 0);

    EXPECT_FALSE(
        rook->isValidMove(0, 0, 1, 0, board)
    );
}


TEST(RookTest, CanCaptureEnemyPiece)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wR", "."},
        {"bP", "."}
    };

    Board board(layout);

    Piece* rook = board.getCell(0, 0);

    EXPECT_TRUE(
        rook->isValidMove(0, 0, 1, 0, board)
    );
}


TEST(QueenTest, CannotMoveThroughPiece)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wQ", "."},
        {"wP", "."},
        {".", "."}
    };

    Board board(layout);

    Piece* queen = board.getCell(0, 0);

    EXPECT_FALSE(
        queen->isValidMove(0, 0, 2, 0, board)
    );
}


TEST(QueenTest, CanCaptureEnemyPiece)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wQ", "."},
        {"bP", "."}
    };

    Board board(layout);

    Piece* queen = board.getCell(0, 0);

    EXPECT_TRUE(
        queen->isValidMove(0, 0, 1, 0, board)
    );
}
TEST(PawnTest, WhitePawnMovesForward)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"wP", "."},
        {".", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_TRUE(
        pawn->isValidMove(1, 0, 0, 0, board)
    );
}


TEST(PawnTest, BlackPawnMovesForward)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"bP", "."},
        {".", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_TRUE(
        pawn->isValidMove(1, 0, 2, 0, board)
    );
}


TEST(PawnTest, WhitePawnCannotMoveBackward)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"wP", "."},
        {".", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 2, 0, board)
    );
}


TEST(PawnTest, BlackPawnCannotMoveBackward)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"bP", "."},
        {".", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 0, 0, board)
    );
}


TEST(PawnTest, PawnCannotMoveTwoSquares)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {".", "."},
        {"wP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(2, 0);

    EXPECT_FALSE(
        pawn->isValidMove(2, 0, 0, 0, board)
    );
}


TEST(PawnTest, PawnCannotMoveSideways)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"wP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 1, 1, board)
    );
}


TEST(PawnTest, WhitePawnCanCaptureEnemyDiagonally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "bP"},
        {"wP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_TRUE(
        pawn->isValidMove(1, 0, 0, 1, board)
    );
}


TEST(PawnTest, BlackPawnCanCaptureEnemyDiagonally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "." , "."},
        {".", "bP", "."},
        {"wP", ".", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 1);

    EXPECT_TRUE(
        pawn->isValidMove(1, 1, 2, 0, board)
    );
}

TEST(PawnTest, PawnCannotCaptureForward)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"wP", "."},
        {"bP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 2, 0, board)
    );
}


TEST(PawnTest, PawnCannotCaptureFriendlyPieceDiagonally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "wP"},
        {"wP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 0, 1, board)
    );
}


TEST(PawnTest, PawnCannotCaptureEmptyDiagonalCell)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"wP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 0, 1, board)
    );
}


TEST(PawnTest, PawnCannotMoveForwardIntoOccupiedCell)
{
    std::vector<std::vector<std::string>> layout =
    {
        {".", "."},
        {"wP", "."},
        {"wP", "."}
    };

    Board board(layout);

    Piece* pawn = board.getCell(1, 0);

    EXPECT_FALSE(
        pawn->isValidMove(1, 0, 2, 0, board)
    );
}
TEST(RookTest, CanMoveHorizontally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wR", ".", "."}
    };

    Board board(layout);

    Piece* rook = board.getCell(0,0);

    EXPECT_TRUE(
        rook->isValidMove(0,0,0,2,board)
    );
}
TEST(RookTest, CannotMoveDiagonally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wR",".","."},
        {".",".","."},
        {".",".","."}
    };

    Board board(layout);

    Piece* rook = board.getCell(0,0);

    EXPECT_FALSE(
        rook->isValidMove(0,0,2,2,board)
    );
}
TEST(BishopTest, CanMoveDiagonally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wB",".","."},
        {".",".","."},
        {".",".","."}
    };

    Board board(layout);

    Piece* bishop = board.getCell(0,0);

    EXPECT_TRUE(
        bishop->isValidMove(0,0,2,2,board)
    );
}
TEST(QueenTest, CanMoveHorizontally)
{
    std::vector<std::vector<std::string>> layout =
    {
        {"wQ",".","."}
    };

    Board board(layout);

    Piece* queen = board.getCell(0,0);

    EXPECT_TRUE(
        queen->isValidMove(0,0,0,2,board)
    );
}