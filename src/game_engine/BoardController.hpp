#pragma once

#include "model/Board.hpp"
#include "model/Piece.hpp"
#include "model/Position.hpp"
#include "model/Constants.hpp"
#include <memory>

class BoardController
{
public:
    explicit BoardController(Board& board);

    /// מבצע מהלך על הלוח:
    ///   - לוקח את piece מהמקור (from)
    ///   - אם יש כלי ביעד (to) — לוכד אותו (מסיר מהלוח, state=Captured)
    ///   - ממקם piece ביעד
    ///   - מחזיר unique_ptr<Piece> שנלכד (nullptr אם לא היה) — הקורא בודק King, ואז משחרר
    ///   - piece אחרי המהלך במצב Idle
    std::unique_ptr<Piece> executeMove(Piece& piece, Position from, Position to);

    /// המרת כלי לסוג אחר (הכתרה):
    ///   - מסיר את ה-piece מהלוח ומוחק
    ///   - יוצר כלי חדש מסוג newType באותו תא ובאותו צבע
    ///   - מחזיר raw observing pointer לכלי החדש (הבעלות נשארת ב-Board)
    Piece* promotePiece(Piece* piece, Position at, PieceType newType);

private:
    Board& board_;
};