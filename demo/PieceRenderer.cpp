#include "PieceRenderer.hpp"
#include "DemoConfig.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PieceRenderer::PieceRenderer(const AssetManager& assets)
    : assets_(&assets)
{
}

cv::Point PieceRenderer::computePiecePixel(const PieceInfo& info, int cellSize) const
{
    const int half = cellSize / 2;
    const int baseX = info.cell.col * cellSize + half;
    const int baseY = info.cell.row * cellSize + half;

    switch (info.state)
    {
    case PieceState::Moving:
    {
        if (info.targetCell.has_value() && info.progress > 0.0)
        {
            double p = info.progress;
            int fromX = info.cell.col * cellSize + half;
            int fromY = info.cell.row * cellSize + half;
            int toX = info.targetCell->col * cellSize + half;
            int toY = info.targetCell->row * cellSize + half;
            return {
                static_cast<int>(fromX + (toX - fromX) * p),
                static_cast<int>(fromY + (toY - fromY) * p)
            };
        }
        return {baseX, baseY};
    }
    case PieceState::Jumping:
    {
        double p = info.progress;
        if (p > 0.0)
        {
            int arcOffset = static_cast<int>(
                -std::sin(p * M_PI) * DemoConfig::JUMP_HEIGHT_PX);
            return {baseX, baseY + arcOffset};
        }
        return {baseX, baseY};
    }
    case PieceState::Idle:
    case PieceState::long_rest: 
    case PieceState::Short_rest: 
    case PieceState::Captured:
    default:
        return {baseX, baseY};
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

        try
    {
        resized.draw_on(canvas, topX, topY);
    }
    catch (const std::exception&)
    {
        // fallback: manual alpha blending for 4-channel canvases
        cv::Mat& canvasMat = canvas.get_mat();
        const cv::Mat& spriteMat = resized.get_mat();
        cv::Rect roi(topX, topY, newW, newH);
        if (roi.x + roi.width <= canvasMat.cols && roi.y + roi.height <= canvasMat.rows)
        {
            cv::Mat roiImg = canvasMat(roi);
            int srcCh = spriteMat.channels();
            int dstCh = roiImg.channels();

            for (int y = 0; y < spriteMat.rows; ++y)
            {
                for (int x = 0; x < spriteMat.cols; ++x)
                {
                    if (srcCh == 4)
                    {
                        const cv::Vec4b& src = spriteMat.at<cv::Vec4b>(y, x);
                        double alpha = src[3] / 255.0;
                        if (dstCh == 4)
                        {
                            cv::Vec4b& dst = roiImg.at<cv::Vec4b>(y, x);
                            dst[0] = static_cast<uchar>(dst[0] * (1.0 - alpha) + src[0] * alpha);
                            dst[1] = static_cast<uchar>(dst[1] * (1.0 - alpha) + src[1] * alpha);
                            dst[2] = static_cast<uchar>(dst[2] * (1.0 - alpha) + src[2] * alpha);
                            dst[3] = 255;
                        }
                        else
                        {
                            cv::Vec3b& dst = roiImg.at<cv::Vec3b>(y, x);
                            dst[0] = static_cast<uchar>(dst[0] * (1.0 - alpha) + src[0] * alpha);
                            dst[1] = static_cast<uchar>(dst[1] * (1.0 - alpha) + src[1] * alpha);
                            dst[2] = static_cast<uchar>(dst[2] * (1.0 - alpha) + src[2] * alpha);
                        }
                    }
                    else
                    {
                        if (dstCh == 4)
                        {
                            const cv::Vec3b& src = spriteMat.at<cv::Vec3b>(y, x);
                            cv::Vec4b& dst = roiImg.at<cv::Vec4b>(y, x);
                            dst[0] = src[0];
                            dst[1] = src[1];
                            dst[2] = src[2];
                            dst[3] = 255;
                        }
                        else
                        {
                            spriteMat.copyTo(roiImg);
                            return;
                        }
                    }
                }
            }
        }
    }

}

void PieceRenderer::drawPieces(Img& canvas, const GameSnapshot& snapshot, int cellSize,
                                AnimationManager& animMgr)
{
    for (const auto& info : snapshot.pieces)
    {
        if (info.state == PieceState::Captured)
            continue;

        const AnimatedSprite* sprite = assets_->getSprite(info.kind, info.color);
        if (!sprite)
            continue;

        cv::Point pixel = computePiecePixel(info, cellSize);

        // שליפת ה-frame המתאים מ-AnimatedSprite לפי state + frame index
        std::string stateName = AnimatedSprite::pieceStateToFolder(
            static_cast<int>(info.state));
        
        int frameIndex = 0;
        if (info.state == PieceState::Moving || info.state == PieceState::Jumping)
        {
            // הערך את ה-frame index מ-AnimMgr
            frameIndex = animMgr.getFrameIndex(info.pieceId);
        }
        
        if (sprite->hasState(stateName) && 
            frameIndex < sprite->getFrameCount(stateName))
        {
            const Img& frameImg = sprite->getFrame(stateName, frameIndex);
            drawSpriteCentered(canvas, frameImg, pixel, cellSize);
        }
    }
}
