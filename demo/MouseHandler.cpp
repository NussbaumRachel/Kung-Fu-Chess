#include "MouseHandler.hpp"
#include "DemoConfig.hpp"
#include "GameController.hpp"
#include <iostream>

MouseHandler::MouseHandler(int cellSize, ClickCallback onCellClick)
    : cellSize_(cellSize), callback_(std::move(onCellClick)) {}

void MouseHandler::attach(const std::string& windowName)
{
    Img::set_mouse_callback(windowName, onMouse, this);
    std::cout << "[MouseHandler] Attached to: " << windowName << '\n';
}

void MouseHandler::onMouse(int event, int x, int y, int flags, void* userdata)
{
    if (event != cv::EVENT_LBUTTONDOWN) return;

    auto* self = static_cast<MouseHandler*>(userdata);
    if (!self || !self->callback_) return;

    Position cell = GameController::pixelsToCell(x, y, self->cellSize_);
    if (cell.row < 0 || cell.row >= DemoConfig::BOARD_ROWS ||
        cell.col < 0 || cell.col >= DemoConfig::BOARD_COLS)
        return;

    self->callback_(cell.row, cell.col);
}
