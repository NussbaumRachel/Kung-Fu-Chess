#ifndef GAME_H
#define GAME_H

#include "Board.hpp"

class Game
{
public:
    explicit Game(Board board);

    void click(int pixelX, int pixelY);

    const Board& getBoard() const;

    void printBoard() const;

private:
    Board board_;

    static constexpr int CELL_SIZE = 100;

    int selectedRow_ = -1;
    int selectedCol_ = -1;

    Color currentTurn_ = Color::White;

    int pixelToCell(int pixel) const;

    bool hasPiece(int row, int col) const;

    Color pieceColor(int row, int col) const;

    void movePiece(int toRow, int toCol);

    void switchTurn();
};

#endif
