#pragma once

#include "model/Piece.hpp"
#include "model/Position.hpp"
#include <memory>

class PieceFactory
{
public:
    // יצירת כלי מתאים לפי מחרוזת token (למשל "wK" → King לבן)
    // מקבל גם מיקום התחלתי. מחזיר בעלות לקורא.
    static std::unique_ptr<Piece> createFromToken(const std::string& token, Position startCell);

    // בדיקה סטטית אם token הוא מחרוזת כלי תקינה
    static bool isValidToken(const std::string& token);

    static std::string makePieceToken(Color color, PieceType type);
    static std::string pieceTypeToChar(PieceType type);

};
