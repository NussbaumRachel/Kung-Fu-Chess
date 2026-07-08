#include "Game.hpp"
#include "Piece.hpp"

Game::Game(Board board)
    : board_(std::move(board))
{
}

const Board& Game::getBoard() const
{
    return board_;
}

void Game::printBoard() const
{
    board_.print();
}

int Game::pixelToCell(int pixel) const
{
    return pixel / CELL_SIZE;
}

bool Game::hasPiece(int row, int col) const
{
    return board_.getCell(row, col) != nullptr;
}

Color Game::pieceColor(int row, int col) const
{
    Piece* p = board_.getCell(row, col);
    return p ? p->getColor() : Color::White; // לא אמור לקרות בלי כלי
}

void Game::movePiece(int toRow, int toCol)
{
    Piece* moving = board_.takeCell(selectedRow_, selectedCol_);
    board_.setCell(toRow, toCol, moving);
}

void Game::switchTurn()
{
    currentTurn_ = (currentTurn_ == Color::White) ? Color::Black : Color::White;
}

void Game::click(int pixelX, int pixelY)
{
    int col = pixelToCell(pixelX);
    int row = pixelToCell(pixelY);

    if (!board_.isInsideBoard(row, col))
        return;

    if (selectedRow_ == -1)
    {
        // בחירת כלי
        if (hasPiece(row, col))
        {
            Piece* p = board_.getCell(row, col);
            if (p->getColor() == currentTurn_)
            {
                selectedRow_ = row;
                selectedCol_ = col;
            }
        }
        return;
    }

    if (hasPiece(row, col))
    {
        Piece* target = board_.getCell(row, col);
        if (target->getColor() == pieceColor(selectedRow_, selectedCol_))
        {
            selectedRow_ = row;
            selectedCol_ = col;
            return;
        }
    }

    Piece* selected = board_.getCell(selectedRow_, selectedCol_);
    if (selected && !selected->isValidMove(selectedRow_, selectedCol_, row, col, board_))
    {
        return;
    }

    movePiece(row, col);

    selectedRow_ = -1;
    selectedCol_ = -1;
    switchTurn();
}
