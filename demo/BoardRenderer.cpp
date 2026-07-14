#include "BoardRenderer.hpp"
#include <iostream>

// ════════════════════════════════════════════
//  כלי-עזר סטטיים
// ════════════════════════════════════════════

Position BoardRenderer::pixelsToCell(int pixelX, int pixelY, int cellSize)
{
    int col = std::clamp(pixelX / cellSize, 0, DemoConfig::BOARD_COLS - 1);
    int row = std::clamp(pixelY / cellSize, 0, DemoConfig::BOARD_ROWS - 1);
    return { row, col };
}

cv::Point BoardRenderer::cellToPixelTopLeft(const Position& cell, int cellSize)
{
    return { cell.col * cellSize, cell.row * cellSize };
}

cv::Point BoardRenderer::cellToPixelCenter(const Position& cell, int cellSize)
{
    return { cell.col * cellSize + cellSize / 2,
             cell.row * cellSize + cellSize / 2 };
}

// ════════════════════════════════════════════
//  טעינת רקע
// ════════════════════════════════════════════

bool BoardRenderer::loadBackground(const std::string& filePath)
{
    try
    {
        background_.read(filePath);
        background_.resize_to(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX);

        // וידוא 3 ערוצים
        if (background_.channels() == 4)
        {
            cv::cvtColor(background_.get_mat(), background_.get_mat(),
                         cv::COLOR_BGRA2BGR);
        }

        backgroundLoaded_ = true;
        std::cout << "[BoardRenderer] Background loaded: " << filePath << '\n';
        return true;
    }
    catch (const std::exception&)
    {
        std::cout << "[BoardRenderer] Cannot load " << filePath
                  << " — using checkerboard.\n";
        backgroundLoaded_ = false;
        return false;
    }
}

// ════════════════════════════════════════════
//  ציור לוח שחמט
// ════════════════════════════════════════════

void BoardRenderer::drawCheckerboard(Img& canvas) const
{
    for (int r = 0; r < DemoConfig::BOARD_ROWS; ++r)
    {
        for (int c = 0; c < DemoConfig::BOARD_COLS; ++c)
        {
            cv::Point tl = cellToPixelTopLeft({r, c}, DemoConfig::CELL_SIZE);
            bool isLight = ((r + c) % 2 == 0);
            cv::Vec3b color = isLight ? DemoConfig::CELL_LIGHT : DemoConfig::CELL_DARK;

            canvas.fill_rect(tl.x, tl.y, DemoConfig::CELL_SIZE, DemoConfig::CELL_SIZE,
                             cv::Scalar(color[0], color[1], color[2]));
        }
    }
}

// ════════════════════════════════════════════
//  Highlight
// ════════════════════════════════════════════

void BoardRenderer::drawCellHighlight(Img& canvas, const Position& cell,
                                      const cv::Vec3b& color, double alpha) const
{
    cv::Point tl = cellToPixelTopLeft(cell, DemoConfig::CELL_SIZE);

    Img overlay = Img::create(DemoConfig::CELL_SIZE, DemoConfig::CELL_SIZE, 3);
    overlay.fill_rect(0, 0, DemoConfig::CELL_SIZE, DemoConfig::CELL_SIZE,
                      cv::Scalar(color[0], color[1], color[2]));

    // שימוש ב-blend_overlay: מיזוג על גבי canvas
    // אבל blend_overlay עובד על כל התמונה — נשתמש ב-cv::addWeighted על ROI
    cv::Mat& canvasMat = canvas.get_mat();
    cv::Rect roi(tl.x, tl.y, DemoConfig::CELL_SIZE, DemoConfig::CELL_SIZE);
    cv::Mat roiImg = canvasMat(roi);
    cv::addWeighted(overlay.get_mat(), alpha, roiImg, 1.0 - alpha, 0, roiImg);
}

// ════════════════════════════════════════════
//  draw
// ════════════════════════════════════════════

// void BoardRenderer::draw(Img& canvas,
//                          const std::optional<Position>& selectedCell,
//                          const std::vector<Position>& highlightedCells) const
// {
//     if (canvas.width() != DemoConfig::BOARD_WIDTH_PX ||
//         canvas.height() != DemoConfig::BOARD_HEIGHT_PX)
//     {
//         canvas = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 3);
//     }

//     if (backgroundLoaded_)
//     {
//         canvas = background_.clone();
//     }
//     else
//     {
//         drawCheckerboard(canvas);
//     }

//     for (const auto& cell : highlightedCells)
//         drawCellHighlight(canvas, cell, DemoConfig::HIGHLIGHT, 0.3);

//     if (selectedCell.has_value())
//         drawCellHighlight(canvas, *selectedCell, DemoConfig::SELECTED, 0.4);
// }
