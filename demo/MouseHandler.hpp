#pragma once

#include "img.hpp"
#include <functional>
#include <string>
#include "model/Position.hpp"

/// טיפול אירועי עכבר. עובד עם Img (דרכה עוברים ל-OpenCV).
class MouseHandler
{
public:
    using ClickCallback = std::function<void(int row, int col)>;

    MouseHandler(int cellSize, ClickCallback onCellClick);

    /// נרשם דרך Img::set_mouse_callback
    void attach(const std::string& windowName);

    static void onMouse(int event, int x, int y, int flags, void* userdata);

    int cellSize() const { return cellSize_; }

private:
    int           cellSize_;
    ClickCallback callback_;
};
