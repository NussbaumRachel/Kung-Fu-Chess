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

// ============================================================
// טסטים לחייל מורחב: צעד כפול + הכתרה
// ============================================================

// חייל לבן יכול לזוז 2 צעדים קדימה משורת הפתיחה
TEST(PawnTest, WhitePawnDoubleStepFromStartRow)
{
    Board board({
        {".", ".", "."},
        {".", ".", "."},
        {".", ".", "."},
        {"wP", ".", "."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(3, 0);   // בוחר wP שורה 3 (שורת פתיחה)
    engine.handleCellClick(1, 0);   // זז 2 צעדים קדימה

    engine.advanceTime(2000);

    EXPECT_NE(engine.getBoard().getCell(1, 0), nullptr);
    EXPECT_EQ(engine.getBoard().getCell(3, 0), nullptr);
    EXPECT_EQ(engine.getBoard().getCell(2, 0), nullptr);   // התא האמצעי ריק
}

// חייל שחור יכול לזוז 2 צעדים קדימה משורת הפתיחה
TEST(PawnTest, BlackPawnDoubleStepFromStartRow)
{
    Board board({
        {"bP", ".", "."},
        {".", ".", "."},
        {".", ".", "."},
        {".", ".", "."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(0, 0);   // בוחר bP שורה 0 (שורת פתיחה)
    engine.handleCellClick(2, 0);   // זז 2 צעדים קדימה

    engine.advanceTime(2000);

    EXPECT_NE(engine.getBoard().getCell(2, 0), nullptr);
    EXPECT_EQ(engine.getBoard().getCell(0, 0), nullptr);
}

// צעד כפול חסום — יש כלי בתא האמצעי
TEST(PawnTest, DoubleStepBlockedByMiddlePiece)
{
    Board board({
        {".", ".", "."},
        {".", ".", "."},
        {"bP", ".", "."},           // אויב חוסם את התא האמצעי
        {"wP", ".", "."}
    });

    GameEngine engine(std::move(board));

    // בוחר wP ומנסה לזוז ל-(1,0) — חסום
    engine.handleCellClick(3, 0);
    engine.handleCellClick(1, 0);

    // הצעד אמור להיכשל — state חוזר ל-WAITING_SELECTION
    EXPECT_EQ(engine.getState(), GameState::WAITING_SELECTION);
    EXPECT_NE(engine.getBoard().getCell(3, 0), nullptr);  // החייל לא זז
}

// צעד כפול — לא עובד מחוץ לשורת הפתיחה
TEST(PawnTest, DoubleStepOnlyFromStartRow)
{
    Board board({
        {".", ".", "."},
        {".", ".", "."},
        {"wP", ".", "."},           // wP בשורה 2 — לא שורת פתיחה
        {".", ".", "."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(2, 0);
    engine.handleCellClick(0, 0);   // מנסה לזוז 2 צעדים

    EXPECT_EQ(engine.getState(), GameState::WAITING_SELECTION);  // נכשל
    EXPECT_NE(engine.getBoard().getCell(2, 0), nullptr);         // החייל נשאר
}

// חייל לבן מגיע לשורה 0 — מוכתר למלכה
TEST(PawnTest, WhitePawnPromotesToQueenAtLastRow)
{
    Board board({
        {".", ".", "."},
        {"wP", ".", "."},
        {".", ".", "."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(1, 0);   // בוחר wP
    engine.handleCellClick(0, 0);   // מגיע לשורה 0

    engine.advanceTime(1000);

    // וידוא: יש כלי ב-(0,0)
    Piece* promoted = engine.getBoard().getCell(0, 0);
    ASSERT_NE(promoted, nullptr);

    // וידוא: זה עכשיו מלכה, לא חייל
    EXPECT_EQ(promoted->getType(), PieceType::Queen);
    EXPECT_EQ(promoted->getColor(), Color::White);
}

// חייל שחור מגיע לשורה אחרונה — מוכתר למלכה
TEST(PawnTest, BlackPawnPromotesToQueenAtLastRow)
{
    Board board({
        {".", ".", "."},
        {"bP", ".", "."},
        {".", ".", "."}
    });

    GameEngine engine(std::move(board));

    engine.handleCellClick(1, 0);   // בוחר bP
    engine.handleCellClick(2, 0);   // צעד בודד קדימה = שורה אחרונה בלוח 3-שורות

    engine.advanceTime(1000);

    Piece* promoted = engine.getBoard().getCell(2, 0);
    ASSERT_NE(promoted, nullptr);
    EXPECT_EQ(promoted->getType(), PieceType::Queen);
    EXPECT_EQ(promoted->getColor(), Color::Black);
}

// הכתרה: החייל מתחלף, ה-id שונה (Piece חדש)
TEST(PawnTest, PromotedPieceHasNewId)
{
    Board board({
        {".", ".", "."},
        {"wP", ".", "."},
        {".", ".", "."}
    });

    GameEngine engine(std::move(board));

    // תפוס את ה-id של החייל לפני התזוזה
    Piece* pawnBefore = engine.getBoard().getCell(1, 0);
    int pawnId = pawnBefore->getId();

    engine.handleCellClick(1, 0);
    engine.handleCellClick(0, 0);
    engine.advanceTime(1000);

    Piece* after = engine.getBoard().getCell(0, 0);
    ASSERT_NE(after, nullptr);
    EXPECT_NE(after->getId(), pawnId);  // id חדש — Piece חדש
}
