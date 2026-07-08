#ifndef BOARD_H
#define BOARD_H

#include "types.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

// הכרזה קדימה
class Piece;

// לוח המשחק — אחראי על אחסון הגריד, גישה לתאים, והדפסה
class Board
{
public:
    using Row = std::vector<Piece*>;
    using Grid = std::vector<Row>;

    Board();
    ~Board();

    // העתקה
    Board(const Board& other);
    Board& operator=(const Board& other);

    // העברה
    Board(Board&& other) noexcept;
    Board& operator=(Board&& other) noexcept;

    // בונה לוח מתוך גריד (לוקח בעלות על ה-piece*)
    explicit Board(const std::vector<std::vector<std::string>>& stringGrid);

    // בדיקה אם הלוח ריק
    bool isEmpty() const;

    // הדפסת הלוח
    void print() const;

    // גישה לגריד הגולמי
    const Grid& getGrid() const;

    // גישה לתא בודד (מחזיר nullptr אם ריק)
    Piece* getCell(int row, int col) const;

    // שינוי תא בודד (לוקח בעלות, nullptr = ריק)
    void setCell(int row, int col, Piece* piece);

    // בדיקה אם תא ריק
    bool isEmptyCell(int row, int col) const;

    // בדיקה אם מיקום נמצא בתוך גבולות הלוח
    bool isInsideBoard(int row, int col) const;

    // מספר שורות ועמודות
    int rowCount() const;
    int colCount() const;

private:
    Grid grid_;

    void clear();
};

#endif
