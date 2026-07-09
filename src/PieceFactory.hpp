#pragma once

#include "Piece.hpp"
#include "Position.hpp"

class PieceFactory
{
public:
    // יצירת כלי מתאים לפי מחרוזת token (למשל "wK" → King לבן)
    // מקבל גם מיקום התחלתי
    static Piece* createFromToken(const std::string& token, Position startCell);

    // בדיקה סטטית אם token הוא מחרוזת כלי תקינה
    static bool isValidToken(const std::string& token);
};
