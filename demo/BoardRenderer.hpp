#pragma once

#include "img.hpp"
#include <optional>
#include <vector>

#include "Position.hpp"
#include "DemoConfig.hpp"

/// מצייר לוח (רקע + הדגשות). עובד עם Img.
class BoardRenderer
{
public:
    BoardRenderer() = default;

    bool loadBackground(const std::string& filePath);

    void draw(Img& canvas,
              const std::optional<Position>& selectedCell,
              const std::vector<Position>& highlightedCells) const;

    static Position  pixelsToCell(int pixelX, int pixelY, int cellSize);
    static cv::Point cellToPixelTopLeft(const Position& cell, int cellSize);
    static cv::Point cellToPixelCenter(const Position& cell, int cellSize);

private:
    Img  background_;
    bool backgroundLoaded_ = false;

    void drawCheckerboard(Img& canvas) const;
    void drawCellHighlight(Img& canvas, const Position& cell,
                           const cv::Vec3b& color, double alpha) const;
};
