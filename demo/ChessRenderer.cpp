#include "ChessRenderer.hpp"
#include <iostream>
#include <filesystem>

ChessRenderer::ChessRenderer()
    : windowName_(DemoConfig::WINDOW_NAME)
{
    assets_ = std::make_unique<AssetManager>();
}

bool ChessRenderer::initialize(const std::string& assetsPath)
{
    // ── לוח ──
    std::string boardPath = assetsPath + "/board.png";
    bool boardOk = assets_->loadBoard(boardPath);

    // ── כלים ──
    std::string piecesPath = assetsPath + "/pieces";
    bool piecesOk = assets_->loadAllPieces(piecesPath);

    // ── BoardRenderer — אם יש board.png, העבר את התמונה
    if (boardOk)
    {
        boardRenderer_.loadBackground(boardPath);
    }
    canvas_ = Img::create(DemoConfig::CANVAS_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 4);

    // PieceRenderer מאותחל פעם אחת, אחרי שה-AssetManager מוכן
    pieceRenderer_ = std::make_unique<PieceRenderer>(*assets_);

    // ── חלון ──
    cv::namedWindow(windowName_, cv::WINDOW_AUTOSIZE);

    std::cout << "[ChessRenderer] Initialized. Board: "
              << (boardOk ? "loaded" : "FALLBACK")
              << ", Pieces: " << (piecesOk ? "loaded" : "FALLBACK")
              << '\n';
    return true;
}

void ChessRenderer::setClickCallback(MouseHandler::ClickCallback callback)
{
    mouseHandler_ = std::make_unique<MouseHandler>(
        DemoConfig::CELL_SIZE, std::move(callback));
}

void ChessRenderer::attachMouse()
{
    if (mouseHandler_)
        mouseHandler_->attach(windowName_);
}

// void ChessRenderer::render(const GameSnapshot& snapshot)
// {
//     canvas_.fill_rect(0, 0, DemoConfig::CANVAS_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX,
//                       cv::Scalar(50, 50, 50, 255));  // grey background

//     std::optional<Position> selectedCell = snapshot.selectedCell;
//     std::vector<Position> highlightedCells;

//     Img boardCanvas = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 4);    boardRenderer_.draw(boardCanvas, selectedCell, highlightedCells);
//     pieceRenderer_->drawPieces(boardCanvas, snapshot, DemoConfig::CELL_SIZE, animMgr_);
//     boardCanvas.draw_on(canvas_, DemoConfig::PANEL_WIDTH_PX, 0);
//     drawMoveHistoryPanel(snapshot);
//     drawScoreHUD(snapshot);

//     if (snapshot.gameOver)
//         drawGameOverlay(snapshot);
// }
void ChessRenderer::render(const GameSnapshot& snapshot)
{

    canvas_.fill_rect(0, 0, DemoConfig::CANVAS_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX,
                      cv::Scalar(50, 50, 50, 255));

    std::optional<Position> selectedCell = snapshot.selectedCell;
    std::vector<Position> highlightedCells;

    Img boardCanvas = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 4);

    boardRenderer_.draw(boardCanvas, selectedCell, highlightedCells);

    pieceRenderer_->drawPieces(boardCanvas, snapshot, DemoConfig::CELL_SIZE, animMgr_);

    boardCanvas.draw_on(canvas_, DemoConfig::PANEL_WIDTH_PX, 0);

    drawMoveHistoryPanel(snapshot);

    drawScoreHUD(snapshot);

    if (snapshot.gameOver)
        drawGameOverlay(snapshot);

}


void ChessRenderer::display()
{
    if (canvas_.is_loaded())
        Img::show_window(windowName_, canvas_);
}

bool ChessRenderer::isWindowOpen() const
{
    return Img::get_window_property(windowName_, cv::WND_PROP_VISIBLE) >= 0;
}

void ChessRenderer::drawGameOverlay(const GameSnapshot& snapshot)
{
    std::string text = "Game Over!";
    if (snapshot.winner.has_value())
        text += (*snapshot.winner == Color::White) ? " White Wins!" : " Black Wins!";

    Img dark = Img::create(DemoConfig::CANVAS_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 4);
    dark.fill_rect(0, 0, DemoConfig::CANVAS_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX,
                   cv::Scalar(0, 0, 0, 255));
    cv::addWeighted(dark.get_mat(), 0.5, canvas_.get_mat(), 0.5, 0, canvas_.get_mat());

    int baseline = 0;
    cv::Size textSz = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX,
                                      DemoConfig::OVERLAY_FONT_SCALE,
                                      DemoConfig::OVERLAY_THICKNESS, &baseline);
    int tx = (DemoConfig::CANVAS_WIDTH_PX - textSz.width) / 2;
    int ty = (DemoConfig::BOARD_HEIGHT_PX + textSz.height) / 2;

    canvas_.put_text(text, tx, ty, DemoConfig::OVERLAY_FONT_SCALE,
                     cv::Scalar(255, 255, 255), DemoConfig::OVERLAY_THICKNESS);
}
void ChessRenderer::drawScoreHUD(const GameSnapshot& snapshot)
{
    int barHeight = 32;
    // Semi-transparent dark bar at the top
    cv::Mat roi = canvas_.get_mat()(cv::Rect(0, 0, DemoConfig::CANVAS_WIDTH_PX, barHeight));
    cv::Mat overlay = roi.clone();
    overlay.setTo(cv::Scalar(0, 0, 0, 255));
    cv::addWeighted(overlay, 0.55, roi, 0.45, 0, roi);

    std::string scoreText = "White: " + std::to_string(snapshot.whiteScore)
                          + "    Black: " + std::to_string(snapshot.blackScore);

    int baseline = 0;
    cv::Size textSz = cv::getTextSize(scoreText, cv::FONT_HERSHEY_SIMPLEX,
                                      0.6, 1, &baseline);
    int tx = (DemoConfig::BOARD_WIDTH_PX - textSz.width) / 2;
    int ty = (barHeight + textSz.height) / 2;

    canvas_.put_text(scoreText, tx, ty, 0.6,
                     cv::Scalar(255, 255, 255), 1);
}
// static helper — piece type to algebraic symbol
static std::string pieceSymbol(PieceType t)
{
    switch (t) {
        case PieceType::King:   return "K";
        case PieceType::Queen:  return "Q";
        case PieceType::Rook:   return "R";
        case PieceType::Bishop: return "B";
        case PieceType::Knight: return "N";
        case PieceType::Pawn:   return "";
        default:                return "?";
    }
}

static std::string posToStr(Position p)
{
    char col = 'a' + (char)p.col;
    char row = '8' - (char)p.row;
    return std::string(1, col) + std::string(1, row);
}

void ChessRenderer::drawMoveHistoryPanel(const GameSnapshot& snapshot)
{
    const int panelW = DemoConfig::PANEL_WIDTH_PX;
    const int boardH = DemoConfig::BOARD_HEIGHT_PX;
    const int topMargin = 36;  // below the score HUD

    // ── Left panel: Black ──
    {
        // Background
        canvas_.fill_rect(0, 0, panelW, boardH, cv::Scalar(40, 40, 40, 255));

        // Header
        canvas_.put_text("Black", 8, topMargin - 8, 0.5, cv::Scalar(200, 200, 200), 1);

        int y = topMargin + 12;
        int maxVisible = 18;  // how many moves fit
        int startIdx = 0;
        if ((int)snapshot.blackMoves.size() > maxVisible)
            startIdx = (int)snapshot.blackMoves.size() - maxVisible;

        for (int i = startIdx; i < (int)snapshot.blackMoves.size() && y < boardH - 4; ++i)
        {
            const auto& m = snapshot.blackMoves[i];

            char buf[64];
            snprintf(buf, sizeof(buf), "%02d:%02d.%03d",
                     m.minutes, m.seconds, m.milliseconds);
            std::string timeStr(buf);
            std::string pieceStr = pieceSymbol(m.pieceType);
            std::string fromStr = posToStr(m.from);
            std::string toStr   = posToStr(m.to);
            std::string line;

            if (m.isJump)
                line = timeStr + " J " + posToStr(m.to);
            else
                line = timeStr + " " + pieceStr + " " + fromStr + "-" + toStr;

            if (m.isCapture)
                line += " x";

            canvas_.put_text(line, 6, y, 0.35, cv::Scalar(220, 220, 220), 1);
            y += 14;
        }
    }

    // ── Right panel: White ──
    {
        int rightX = DemoConfig::BOARD_WIDTH_PX + panelW;

        // Background
        canvas_.fill_rect(rightX, 0, panelW, boardH, cv::Scalar(40, 40, 40, 255));

        // Header
        canvas_.put_text("White", rightX + 8, topMargin - 8, 0.5, cv::Scalar(200, 200, 200), 1);

        int y = topMargin + 12;
        int maxVisible = 18;
        int startIdx = 0;
        if ((int)snapshot.whiteMoves.size() > maxVisible)
            startIdx = (int)snapshot.whiteMoves.size() - maxVisible;

        for (int i = startIdx; i < (int)snapshot.whiteMoves.size() && y < boardH - 4; ++i)
        {
            const auto& m = snapshot.whiteMoves[i];

            char buf[64];
            snprintf(buf, sizeof(buf), "%02d:%02d.%03d",
                     m.minutes, m.seconds, m.milliseconds);
            std::string timeStr(buf);
            std::string pieceStr = pieceSymbol(m.pieceType);
            std::string fromStr = posToStr(m.from);
            std::string toStr   = posToStr(m.to);
            std::string line;

            if (m.isJump)
                line = timeStr + " J " + posToStr(m.to);
            else
                line = timeStr + " " + pieceStr + " " + fromStr + "-" + toStr;

            if (m.isCapture)
                line += " x";

            canvas_.put_text(line, rightX + 6, y, 0.35, cv::Scalar(220, 220, 220), 1);
            y += 14;
        }
    }
}
