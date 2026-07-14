#include "PieceRenderer.hpp"
#include "DemoConfig.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PieceRenderer::PieceRenderer(const AssetManager& assets)
    : assets_(&assets) {}

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
    if (!sprite.is_loaded()) return;

    int margin     = static_cast<int>(cellSize * 0.1);
    int spriteSize = cellSize - margin * 2;

    Img resized = sprite.clone();
    // resize לגודל תא — שמירה על aspect ratio
    double scaleX = static_cast<double>(spriteSize) / resized.width();
    double scaleY = static_cast<double>(spriteSize) / resized.height();
    double scale  = std::min(scaleX, scaleY);
    int newW = static_cast<int>(resized.width()  * scale);
    int newH = static_cast<int>(resized.height() * scale);
    resized.resize_to(newW, newH);

    int topX = center.x - newW / 2;
    int topY = center.y - newH / 2;

    if (topX < 0) topX = 0;
    if (topY < 0) topY = 0;
    if (topX + newW > canvas.width())  topX = canvas.width()  - newW;
    if (topY + newH > canvas.height()) topY = canvas.height() - newH;

    try
    {
        resized.draw_on(canvas, topX, topY);
    }
    catch (const std::exception&)
    {
        // fallback: העתקה ישירה
        cv::Mat& canvasMat = canvas.get_mat();
        const cv::Mat& spriteMat = resized.get_mat();
        cv::Rect roi(topX, topY, newW, newH);
        if (roi.x + roi.width <= canvasMat.cols && roi.y + roi.height <= canvasMat.rows)
        {
            cv::Mat roiImg = canvasMat(roi);
            if (spriteMat.channels() == 4)
            {
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

void PieceRenderer::drawPieces(Img& canvas, const GameSnapshot& snapshot,
                               int cellSize, AnimationManager& animMgr)
{
    for (const auto& info : snapshot.pieces)
    {
        if (info.state == PieceState::Captured) continue;

        // מעקב תא קודם
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

        // ── שליפת frame מ-AnimatedSprite ──
        const AnimatedSprite* animSprite = assets_->getSprite(info.kind, info.color);
        std::string stateName = AnimatedSprite::pieceStateToFolder(
            static_cast<int>(info.state));

        // חישוב frameIndex: elapsed / (1.0/fps) = elapsed * fps
        double elapsed = animMgr.getElapsed(info.pieceId);
        const Img* frame = nullptr;

        if (animSprite && animSprite->hasState(stateName))
        {
            const auto& cfg = animSprite->getConfig(stateName);
            double fps = cfg.fps > 0 ? cfg.fps : 12.0;
            int frameCount = animSprite->getFrameCount(stateName);
            int frameIdx = 0;
            if (frameCount > 0)
                frameIdx = static_cast<int>(elapsed * fps) % frameCount;

            frame = &animSprite->getFrame(stateName, frameIdx);
        }

        // Fallback
        if (!frame || !frame->is_loaded())
        {
            // ציור fallback (עיגול צבעוני)
            static Img fallback;
            cv::Point pixel = computePiecePixel(info, cellSize);
            // skip — no fallback for now, use old SpriteSheet fallback
            continue;
        }

        // ── ציור ──
        cv::Point pixel = computePiecePixel(info, cellSize);
        drawSpriteCentered(canvas, *frame, pixel, cellSize);

        // ── עדכון AnimationManager ──
        animMgr.update(info.pieceId, static_cast<int>(info.state),
                       DemoConfig::FRAME_DELAY_MS / 1000.0);
    }
}
