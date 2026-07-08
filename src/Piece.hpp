#ifndef PIECE_H
#define PIECE_H

#include "types.hpp"
#include <string>

// הכרזה קדימה
class Board;

// מחלקת בסיס לכל כלי שחמט
class Piece
{
public:
    Piece(Color color, PieceType type);
    virtual ~Piece() = default;

    Color getColor() const;
    PieceType getType() const;

    // מחזירה מחרוזת בת 2 תווים (למשל "wK")
    std::string toString() const;

    // בדיקה אם המהלך חוקי לפי חוקי הכלי
    virtual bool isValidMove(int fromRow, int fromCol,
                             int toRow, int toCol,
                             const Board& board) const = 0;

    // יצירת כלי מתאים לפי מחרוזת token (למשל "wK" → King לבן)
    static Piece* createFromToken(const std::string& token);

    // בדיקה סטטית אם token הוא מחרוזת כלי תקינה (לשימוש בפרסור)
    static bool isValidToken(const std::string& token);

protected:
    Color color_;
    PieceType type_;
};

#endif
