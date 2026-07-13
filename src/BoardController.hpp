#pragma once

#include "Board.hpp"
#include "Piece.hpp"
#include "Position.hpp"
#include "types.hpp"
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

    /// המרת חייל למלכה:
    ///   - מסיר את ה-pawn מהלוח ומוחק
    ///   - יוצר Queen חדשה באותו תא
    ///   - מחזיר raw observing pointer למלכה החדשה (הבעלות נשארת ב-Board)
    Piece* promoteToQueen(Piece* pawn, Position at);

private:
    Board& board_;
};