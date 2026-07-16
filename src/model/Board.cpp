#include "model/Board.hpp"
#include "model/Piece.hpp"
#include "movement/PieceFactory.hpp"

// ──────────────── העתקה ────────────────

void Board::copyFrom(const Board& other)
{
    grid_.resize(other.grid_.size());
    for (size_t r = 0; r < other.grid_.size(); ++r)
    {
        grid_[r].resize(other.grid_[r].size());
        for (size_t c = 0; c < other.grid_[r].size(); ++c)
        {
            if (other.grid_[r][c])
                grid_[r][c] = PieceFactory::createFromToken(
                    other.grid_[r][c]->toString(),
                    {static_cast<int>(r), static_cast<int>(c)});
            else
                grid_[r][c] = nullptr;
        }
    }
}

Board::Board(const Board& other)
{
    copyFrom(other);
}

Board& Board::operator=(const Board& other)
{
    if (this != &other)
    {
        grid_.clear();            // unique_ptr ה-destructors רצים אוטומטית
        copyFrom(other);
    }
    return *this;
}

Board Board::clone() const
{
    return Board(*this);
}

// ──────────────── בנייה מ-stringGrid ────────────────

Board::Board(const std::vector<std::vector<std::string>>& stringGrid)
{
    grid_.resize(stringGrid.size());
    for (size_t r = 0; r < stringGrid.size(); ++r)
    {
        grid_[r].resize(stringGrid[r].size());
        for (size_t c = 0; c < stringGrid[r].size(); ++c)
        {
            const auto& token = stringGrid[r][c];
            if (token == ".")
                grid_[r][c] = nullptr;
            else
                grid_[r][c] = PieceFactory::createFromToken(
                    token,
                    {static_cast<int>(r), static_cast<int>(c)});
        }
    }
}

// ──────────────── גישה לתאים ────────────────

bool Board::isEmpty() const { return grid_.empty(); }

const Board::Grid& Board::getGrid() const { return grid_; }

Piece* Board::getCell(int row, int col) const
{
    return grid_[row][col].get();
}

void Board::setCell(int row, int col, std::unique_ptr<Piece> piece)
{
    grid_[row][col] = std::move(piece);
}

std::unique_ptr<Piece> Board::takeCell(int row, int col)
{
    return std::move(grid_[row][col]);
}

bool Board::isEmptyCell(int row, int col) const
{
    return grid_[row][col] == nullptr;
}

// ──────────────── שאילתות ────────────────

bool Board::isInsideBoard(int row, int col) const
{
    if (grid_.empty()) return false;
    return row >= 0 && row < static_cast<int>(grid_.size()) &&
           col >= 0 && col < static_cast<int>(grid_[0].size());
}

int Board::rowCount() const { return static_cast<int>(grid_.size()); }
int Board::colCount() const { return grid_.empty() ? 0 : static_cast<int>(grid_[0].size()); }

bool Board::hasFriendlyPiece(int row, int col, Color color) const
{
    Piece* piece = getCell(row, col);
    return piece != nullptr && piece->getColor() == color;
}

bool Board::hasEnemyPiece(int row, int col, Color color) const
{
    Piece* piece = getCell(row, col);
    return piece != nullptr && piece->getColor() != color;
}

bool Board::isPathClear(int fromRow, int fromCol,
                        int toRow, int toCol) const
{
    int rowStep = (toRow > fromRow) - (toRow < fromRow);
    int colStep = (toCol > fromCol) - (toCol < fromCol);

    int row = fromRow + rowStep;
    int col = fromCol + colStep;

    while (row != toRow || col != toCol)
    {
        if (!isEmptyCell(row, col) && !(getCell(row, col).getState() == PieceInfo::move))
            return false;

        row += rowStep;
        col += colStep;
    }

    return true;
}

bool Board::hasKing(Position pos) const
{
    if (!isInsideBoard(pos.row, pos.col))
        return false;

    const Piece* piece = grid_[pos.row][pos.col].get();

    return piece != nullptr &&
           piece->getType() == PieceType::King;
}

// ──────────────── הדפסה ────────────────

void Board::print() const
{
    for (size_t i = 0; i < grid_.size(); ++i)
    {
        const auto& row = grid_[i];
        for (size_t j = 0; j < row.size(); ++j)
        {
            if (j > 0) std::cout << ' ';
            if (row[j])
                std::cout << row[j]->toString();
            else
                std::cout << '.';
        }
        std::cout << '\n';
    }
}