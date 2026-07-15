#include "PieceRenderer.hpp"
#include "DemoConfig.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PieceRenderer::PieceRenderer(const SpriteSheet& spriteSheet)
    : spriteSheet_(spriteSheet) {}

cv::Point PieceRenderer::computePiecePixel(const PieceInfo& info, int cellSize) const
{
    const int half  = cellSize / 2;
    const int baseX = info.cell.col * cellSize + half;
    const int baseY = info.cell.row * cellSize + half;

    switch (info.state)
    {
    case PieceState::Moving:
    {
        auto it = previousCells_.find(info.pieceId);
        if (it != previousCells_.end() && info.progress > 0.0)
        {
            double p = info.progress;
            int fromX = it->second.col * cellSize + half;
            int fromY = it->second.row * cellSize + half;
            return {
                static_cast<int>(fromX + (baseX - fromX) * p),
                static_cast<int>(fromY + (baseY - fromY) * p)
            };
        }
        return { baseX, baseY };
    }
    case PieceState::Jumping:
    {
        double p = info.progress;
        if (p > 0.0)
        {
            int arcOffset = static_cast<int>(
                -std::sin(p * M_PI) * DemoConfig::JUMP_HEIGHT_PX);
            return { baseX, baseY + arcOffset };
        }
        return { baseX, baseY };
    }
    case PieceState::Idle:
    case PieceState::Captured:
    default:
        return { baseX, baseY };
    }
}

void PieceRenderer::drawSpriteCentered(Img& canvas, const Img& sprite,
                                       const cv::Point& center, int cellSize) const
{
    int margin     = static_cast<int>(cellSize * 0.1);
    int spriteSize = cellSize - margin * 2;

    // resize
    Img resized = sprite.clone();
    resized.resize_to(spriteSize, spriteSize);

    int topX = center.x - spriteSize / 2;
    int topY = center.y - spriteSize / 2;

    if (topX < 0) topX = 0;
    if (topY < 0) topY = 0;
    if (topX + spriteSize > canvas.width())  topX = canvas.width()  - spriteSize;
    if (topY + spriteSize > canvas.height()) topY = canvas.height() - spriteSize;

    // שימוש ב-draw_on (המתודה המקורית של Img)
    try
    {
        resized.draw_on(canvas, topX, topY);
    }
    catch (const std::exception&)
    {
        // fallback: העתקה ישירה
        cv::Mat& canvasMat = canvas.get_mat();
        const cv::Mat& spriteMat = resized.get_mat();
        cv::Rect roi(topX, topY, spriteSize, spriteSize);
        if (roi.x + roi.width <= canvasMat.cols && roi.y + roi.height <= canvasMat.rows)
        {
            if (spriteMat.channels() == 4)
            {
                cv::Mat roiImg = canvasMat(roi);
                for (int y = 0; y < spriteMat.rows; ++y)
                {
                    for (int x = 0; x < spriteMat.cols; ++x)
                    {
                        const cv::Vec4b& src = spriteMat.at<cv::Vec4b>(y, x);
                        cv::Vec3b& dst = roiImg.at<cv::Vec3b>(y, x);
                        double alpha = src[3] / 255.0;
                        dst[0] = static_cast<uchar>(dst[0] * (1.0 - alpha) + src[0] * alpha);
                        dst[1] = static_cast<uchar>(dst[1] * (1.0 - alpha) + src[1] * alpha);
                        dst[2] = static_cast<uchar>(dst[2] * (1.0 - alpha) + src[2] * alpha);
                    }
                }
            }
            else
            {
                spriteMat.copyTo(roiImg);
            }
        }
    }
}

void PieceRenderer::drawPieces(Img& canvas, const GameSnapshot& snapshot, int cellSize)
{
    for (const auto& info : snapshot.pieces)
    {
        if (info.state == PieceState::Captured)
            continue;

        auto it = previousCells_.find(info.pieceId);
        if (it != previousCells_.end())
        {
            if (!(it->second == info.cell))
                it->second = info.cell;
        }
        else
        {
            previousCells_[info.pieceId] = info.cell;
        }

        if (info.state == PieceState::Idle)
            previousCells_[info.pieceId] = info.cell;

        Img sprite = spriteSheet_.getSprite(info.kind, info.color);
        cv::Point pixel = computePiecePixel(info, cellSize);

        drawSpriteCentered(canvas, sprite, pixel, cellSize);
    }
}
