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

    Piece* movingPiece = board_.getCell(
        selectedRow_,
        selectedCol_
    );

    if (movingPiece == nullptr)
        return;

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

    // בחירת כלי
    if (selectedRow_ == -1)
    {
        if (hasPiece(row, col) && !isPieceMoving(row, col))
        {
            selectedRow_ = row;
            selectedCol_ = col;
        }

        return;
    }

    // בדיקה שהכלי שנבחר עדיין קיים ולא בתנועה
    if (isPieceMoving(selectedRow_, selectedCol_))
    {
        selectedRow_ = -1;
        selectedCol_ = -1;
        return;
    }

    Piece* selectedPiece = board_.getCell(selectedRow_, selectedCol_);

    if (selectedPiece == nullptr)
    {
        selectedRow_ = -1;
        selectedCol_ = -1;
        return;
    }

    // לחיצה על כלי ידידותי אחר מחליפה בחירה
    if (hasPiece(row, col))
    {
        Piece* targetPiece = board_.getCell(row, col);

        if (targetPiece->getColor() == selectedPiece->getColor())
        {
            if (!isPieceMoving(row, col))
            {
                selectedRow_ = row;
                selectedCol_ = col;
            }

            return;
        }
    }

    // בדיקת חוקיות תנועה
    if (!selectedPiece->isValidMove(
            selectedRow_,
            selectedCol_,
            row,
            col,
            board_))
    {
        return;
    }
    if (selectedPiece->getType() != PieceType::Knight)
    {
        if (!board_.isPathClear(
                selectedRow_,
                selectedCol_,
                row,
                col))
        {
            return;
        }
    }
    if (board_.hasFriendlyPiece(row, col, selectedPiece->getColor()))   
    {
        selectedRow_ = -1;
        selectedCol_ = -1;
        return;
    }
    startMove(row, col);

    selectedRow_ = -1;
    selectedCol_ = -1;
}
void Game::update(int milliseconds)
{
    resolveCollisions();

    for(auto it = activeMoves_.begin();
        it != activeMoves_.end();)
    {
        it->update(milliseconds);

        if (it->isCancelled())
        {
            it = activeMoves_.erase(it);
        }
        else if (it->isFinished())
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
        Position from = move.getFrom();
        Position to = move.getTo();

        if((from.row == row && from.col == col) ||
           (to.row == row && to.col == col))
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
void Game::resolveCollisions()
{
    for (size_t i = 0; i < activeMoves_.size(); ++i)
    {
        for (size_t j = i + 1; j < activeMoves_.size(); ++j)
        {
            Move& first = activeMoves_[i];
            Move& second = activeMoves_[j];

            if (first.isCancelled() || second.isCancelled())
                continue;

            Piece* firstPiece = first.getPiece();
            Piece* secondPiece = second.getPiece();

            if (firstPiece == nullptr || secondPiece == nullptr)
                continue;

            if (firstPiece->getColor() == secondPiece->getColor())
                continue;

            if (first.getTo() == second.getTo())
            {
                second.cancel();
            }
        }
    }
}