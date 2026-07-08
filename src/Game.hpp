#ifndef GAME_H
#define GAME_H
#include "Move.hpp"
#include "Board.hpp"
#include <vector>
class Game
{
public:
    explicit Game(Board board);

    void click(int pixelX, int pixelY);

    const Board& getBoard() const;
    void wait(int milliseconds);
    void printBoard() const;
    void update(int milliseconds);
private:
    Board board_;
    std::vector<Move> activeMoves_;
    int currentTime_ = 0;
    static constexpr int CELL_SIZE = 100;

    int selectedRow_ = -1;
    int selectedCol_ = -1;

    int pixelToCell(int pixel) const;

    bool hasPiece(int row, int col) const;

    Color pieceColor(int row, int col) const;

    void startMove(int toRow, int toCol);

    void finishMove(const Move& move);

    bool isPieceMoving(int row, int col) const;

    int calculateMoveTime(
        int fromRow,
        int fromCol,
        int toRow,
        int toCol
    ) const;
    
};

#endif
