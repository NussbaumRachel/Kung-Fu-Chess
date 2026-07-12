#ifndef PIECE_H
#define PIECE_H

#include "types.hpp"
#include "Position.hpp"
#include <string>

// הכרזה קדימה
class Board;

enum class PieceState
{
    Idle,
    Moving,
    Jumping,
    Captured
};

// מחלקת בסיס לכל כלי שחמט
class Piece
{
public:
    Piece(Color color, PieceType type, Position startCell);
    virtual ~Piece() = default;

    int getId() const;
    Color getColor() const;
    PieceType getType() const;
    PieceState getState() const;
    Position getCell() const;

    void setState(PieceState state);
    void setCell(Position cell);

    // מחזירה מחרוזת בת 2 תווים (למשל "wK")
    std::string toString() const;

    // בדיקה אם המהלך חוקי לפי חוקי הכלי
    virtual bool isValidMove(int fromRow, int fromCol,
                             int toRow, int toCol,
                             const Board& board) const = 0;

protected:
    int id_;
    Color color_;
    PieceType type_;
    PieceState state_ = PieceState::Idle;
    Position cell_;

private:
    static int nextId_;
};

#endif
