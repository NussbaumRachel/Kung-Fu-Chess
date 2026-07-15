#ifndef BOARD_H
#define BOARD_H

#include "model/Position.hpp"
#include "model/Constants.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

// הכרזה קדימה
class Piece;

// לוח המשחק — אחראי אחסון הגריד, גישה לתאים, והדפסה
// הבעלים הבלעדי של כל ה-Piece-ים במשחק
class Board
{
public:
    using Row = std::vector<std::unique_ptr<Piece>>;
    using Grid = std::vector<Row>;

    Board() = default;
    ~Board() = default;

    // העתקה — שכפול עמוק (דרך PieceFactory)
    Board(const Board& other);
    Board& operator=(const Board& other);

    // שכפול עמוק (נוחות — מפעיל את בנאי ההעתקה)
    Board clone() const;

    // העברה
    Board(Board&& other) noexcept = default;
    Board& operator=(Board&& other) noexcept = default;

    // בונה לוח מתוך גריד טקסטואלי
    explicit Board(const std::vector<std::vector<std::string>>& stringGrid);

    // בדיקה האם הלוח ריק
    bool isEmpty() const;

    // הדפסת הלוח
    void print() const;

    // גישה לגריד הגולמי (קריאה בלבד)
    const Grid& getGrid() const;

    // גישה לתא בודד — observing pointer, ללא בעלות
    Piece* getCell(int row, int col) const;

    // שינוי תא — מקבל בעלות על piece ; nullptr = ריק
    void setCell(int row, int col, std::unique_ptr<Piece> piece);

    // שליפת כלי מתא — מעביר בעלות לקורא ; nullptr = ריק
    std::unique_ptr<Piece> takeCell(int row, int col);

    // בדיקה האם תא ריק
    bool isEmptyCell(int row, int col) const;

    // בדיקה האם קואורדינטות בתוך גבולות הלוח
    bool isInsideBoard(int row, int col) const;

    // מימדי הלוח
    int rowCount() const;
    int colCount() const;

    // בדיקת שכן — כלי ידידותי / אויב
    bool hasFriendlyPiece(int row, int col, Color color) const;
    bool hasEnemyPiece(int row, int col, Color color) const;

    // בדיקת מסלול פנוי בין שני תאים (לא כולל מקור ויעד)
    bool isPathClear(int fromRow, int fromCol, int toRow, int toCol) const;

    // בדיקה האם בתא מסוים יש מלך
    bool hasKing(Position pos) const;

private:
    Grid grid_;

    // שכפול עמוק — פונקציית עזר פנימית
    void copyFrom(const Board& other);
};

#endif