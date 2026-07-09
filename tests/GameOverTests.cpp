#include <gtest/gtest.h>
#include "GameEngine.hpp"
#include "Board.hpp"

// ============================================================
// טסטים להתנהגות Game Over
// ============================================================

// לכידת מלך האויב מסיימת את המשחק
TEST(GameOverTest, CapturingEnemyKingEndsGame)
{
    // לוח: צריח לבן מול מלך שחור
    Board board({
        {"wR", ".", "bK"}
    });

    GameEngine engine(std::move(board));

    // מהלך: צריח לבן לוכד מלך שחור
    engine.handleCellClick(0, 0);   // בוחר wR
    engine.handleCellClick(0, 2);   // לוכד bK

    engine.advanceTime(2000);       // המתנה לסיום התנועה

    // וידוא: המשחק נגמר
    EXPECT_EQ(engine.getState(), GameState::GAME_OVER);

    // וידוא: הצריח נמצא במקום המלך
    EXPECT_NE(engine.getBoard().getCell(0, 2), nullptr);
    EXPECT_EQ(engine.getBoard().getCell(0, 0), nullptr);

    // וידוא: המלך השחור נלכד
    Piece* piece = engine.getBoard().getCell(0, 2);
    ASSERT_NE(piece, nullptr);
    EXPECT_EQ(piece->getType(), PieceType::Rook);
    EXPECT_EQ(piece->getColor(), Color::White);
}

// מהלך שמנסה ללכוד כלי רגיל (לא מלך) לא מסיים את המשחק
TEST(GameOverTest, CapturingRegularPieceDoesNotEndGame)
{
    Board board({
        {"wR", ".", "bP"}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);   // בוחר wR
    engine.handleCellClick(0, 2);   // לוכד bP (חייל, לא מלך)

    engine.advanceTime(2000);

    // וידוא: המשחק ממשיך כרגיל
    EXPECT_EQ(engine.getState(), GameState::WAITING_SELECTION);
    EXPECT_NE(engine.getBoard().getCell(0, 2), nullptr);
}

// אחרי Game Over — פקודות move עתידיות נדחות
TEST(GameOverTest, MovesIgnoredAfterGameOver)
{
    Board board({
        {"wR", ".", "bK"}
    });

    GameEngine engine(std::move(board));

    // ביצוע מהלך מסיים משחק
    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 2);
    engine.advanceTime(2000);
    EXPECT_EQ(engine.getState(), GameState::GAME_OVER);

    // ניסיון לבחור כלי אחרי GameOver — אמור להיכשל
    // (אם יש עוד כלים על הלוח, אי אפשר להזיז אותם)
    engine.handleCellClick(0, 2);   // מנסה לבחור את הצריח

    // state נשאר GAME_OVER, לא WAITING_TARGET
    EXPECT_EQ(engine.getState(), GameState::GAME_OVER);
}

// Wait לא משפיע אחרי Game Over
TEST(GameOverTest, WaitHasNoEffectAfterGameOver)
{
    Board board({
        {"wR", ".", "bK"}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);
    engine.handleCellClick(0, 2);
    engine.advanceTime(2000);
    EXPECT_EQ(engine.getState(), GameState::GAME_OVER);

    // wait לא אמור לשנות דבר
    engine.advanceTime(5000);
    EXPECT_EQ(engine.getState(), GameState::GAME_OVER);
}

// לכידת מלך ע"י שחור — בודק Winner
TEST(GameOverTest, BlackCapturesWhiteKing)
{
    Board board({
        {"bR", ".", "wK"}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);   // בוחר bR
    engine.handleCellClick(0, 2);   // לוכד wK

    engine.advanceTime(2000);

    EXPECT_EQ(engine.getState(), GameState::GAME_OVER);

    // וידוא Winner ב-Snapshot
    GameSnapshot snap = engine.getSnapshot();
    ASSERT_TRUE(snap.winner.has_value());
    EXPECT_EQ(*snap.winner, Color::Black);
}
