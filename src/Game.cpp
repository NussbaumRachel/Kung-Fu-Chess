#include "Game.hpp"
#include "Piece.hpp"
#include <cmath>

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

void Game::startMove(int toRow, int toCol)
{
    Position from{
        selectedRow_,
        selectedCol_
    };

    Position to{
        toRow,
        toCol
    };


    activeMoves_.emplace_back(
        board_.getCell(selectedRow_, selectedCol_),
        from,
        to,
        calculateMoveTime(
            selectedRow_,
            selectedCol_,
            toRow,
            toCol
        )
    );
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
        if (hasPiece(row, col) &&
            !isPieceMoving(row,col))
        {
            Piece* p = board_.getCell(row, col);
                selectedRow_ = row;
                selectedCol_ = col;
        }
        return;
    }

    if (hasPiece(row, col) &&
        !isPieceMoving(row,col))
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

    startMove(row, col);
    selectedRow_ = -1;
    selectedCol_ = -1;
}
void Game::update(int milliseconds)
{
    for(auto it = activeMoves_.begin();
        it != activeMoves_.end();)
    {
        it->update(milliseconds);

        if(it->isFinished())
        {
            finishMove(*it);
            it = activeMoves_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
void Game::finishMove(const Move& move)
{
    Position from = move.getFrom();
    Position to = move.getTo();


    Piece* piece =
        board_.takeCell(
            from.row,
            from.col
        );


    if(board_.getCell(to.row,to.col))
    {
        delete board_.takeCell(
            to.row,
            to.col
        );
    }


    board_.setCell(
        to.row,
        to.col,
        piece
    );
}
bool Game::isPieceMoving(int row,int col) const
{
    for(const Move& move : activeMoves_)
    {
        Position pos = move.getFrom();

        if(pos.row == row &&
           pos.col == col)
        {
            return true;
        }
    }

    return false;
}
int Game::calculateMoveTime(
    int fromRow,
    int fromCol,
    int toRow,
    int toCol) const
{
    int distance =
        abs(toRow-fromRow) +
        abs(toCol-fromCol);


    return distance * 1000;
}
void Game::wait(int milliseconds)
{
    update(milliseconds);
}