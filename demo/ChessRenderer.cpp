#include "ChessRenderer.hpp"
#include <iostream>
#include <filesystem>

ChessRenderer::ChessRenderer()
    : pieceRenderer_(*assets_)  // זמני — מוחלף ב-initialize
    , windowName_(DemoConfig::WINDOW_NAME)
{
    assets_ = std::make_unique<AssetManager>();
}

bool ChessRenderer::initialize(const std::string& assetsPath)
{
    namespace fs = std::filesystem;

    // ── לוח ──
    std::string boardPath = assetsPath + "/board.png";
    bool boardOk = assets_->loadBoard(boardPath);

    // ── כלים ──
    std::string piecesPath = assetsPath + "/pieces2";
    bool piecesOk = assets_->loadAllPieces(piecesPath);

    // ── BoardRenderer — אם יש board.png, העבר את התמונה
    if (boardOk)
    {
        boardRenderer_.loadBackground(boardPath);
    }
    canvas_ = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 3);

    // PieceRenderer נבנה מחדש עם ה-AssetManager (עכשיו pointer — assignment עובד)
    pieceRenderer_ = PieceRenderer(*assets_);

    // ── חלון ──
    Img::named_window(windowName_);

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

void ChessRenderer::render(const GameSnapshot& snapshot)
{
    std::optional<Position> selectedCell = snapshot.selectedCell;
    std::vector<Position> highlightedCells;

    boardRenderer_.draw(canvas_, selectedCell, highlightedCells);
    pieceRenderer_.drawPieces(canvas_, snapshot, DemoConfig::CELL_SIZE, animMgr_);

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

    Img dark = Img::create(DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX, 3);
    dark.fill_rect(0, 0, DemoConfig::BOARD_WIDTH_PX, DemoConfig::BOARD_HEIGHT_PX,
                   cv::Scalar(0, 0, 0));
    cv::addWeighted(dark.get_mat(), 0.5, canvas_.get_mat(), 0.5, 0, canvas_.get_mat());

    int baseline = 0;
    cv::Size textSz = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX,
                                      DemoConfig::OVERLAY_FONT_SCALE,
                                      DemoConfig::OVERLAY_THICKNESS, &baseline);
    int tx = (DemoConfig::BOARD_WIDTH_PX  - textSz.width)  / 2;
    int ty = (DemoConfig::BOARD_HEIGHT_PX + textSz.height) / 2;

    canvas_.put_text(text, tx, ty, DemoConfig::OVERLAY_FONT_SCALE,
                     cv::Scalar(255, 255, 255), DemoConfig::OVERLAY_THICKNESS);
}
