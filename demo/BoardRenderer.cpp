#include "BoardRenderer.hpp"
#include <iostream>

// ════════════════════════════════════════════
//  כלי-עזר סטטיים
// ════════════════════════════════════════════

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

        // Keep alpha channel for proper transparency compositing

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
//  Highlight
// ════════════════════════════════════════════

void BoardRenderer::drawCellHighlight(Img& canvas, const Position& cell,
                                      const cv::Vec3b& color, double alpha) const
{
    cv::Point tl = cellToPixelTopLeft(cell, DemoConfig::CELL_SIZE);
    int sz = DemoConfig::CELL_SIZE;

    cv::Mat& canvasMat = canvas.get_mat();
    cv::Rect roi(tl.x, tl.y, sz, sz);
    if (roi.x + roi.width > canvasMat.cols || roi.y + roi.height > canvasMat.rows)
        return;

    cv::Mat roiImg = canvasMat(roi);
    for (int y = 0; y < sz; ++y) {
        for (int x = 0; x < sz; ++x) {
            cv::Vec4b& dst = roiImg.at<cv::Vec4b>(y, x);
            double a = alpha;
            dst[0] = static_cast<uchar>(dst[0] * (1.0 - a) + color[0] * a);
            dst[1] = static_cast<uchar>(dst[1] * (1.0 - a) + color[1] * a);
            dst[2] = static_cast<uchar>(dst[2] * (1.0 - a) + color[2] * a);
            // leave dst[3] (alpha) unchanged
        }
    }
}

// ════════════════════════════════════════════
//  draw
// ════════════════════════════════════════════

void BoardRenderer::draw(Img& canvas,
                         const std::optional<Position>& selectedCell,
                         const std::vector<Position>& highlightedCells) const
{
    if (canvas.width() != DemoConfig::BOARD_WIDTH_PX ||
        canvas.height() != DemoConfig::BOARD_HEIGHT_PX)
    {
        canvas = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 4);
    }

    if (backgroundLoaded_)
    {
        canvas = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 4);
        background_.draw_on(canvas, 0, 0);
    }

    for (const auto& cell : highlightedCells)
        drawCellHighlight(canvas, cell, DemoConfig::HIGHLIGHT, 0.3);

    if (selectedCell.has_value())
        drawCellHighlight(canvas, *selectedCell, DemoConfig::SELECTED, 0.4);
}
