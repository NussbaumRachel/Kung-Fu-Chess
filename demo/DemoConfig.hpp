#pragma once

#include "img.hpp"   // Img מביא איתו את OpenCV (cv::Vec3b וכו')
#include <cstdint>

/// כל הקבועים של פרויקט הדמו במקום אחד.
/// ריכוז הקבועים מקל על התאמה ושינוי מהירים — אין "מספרי הקסם" מפוזרים.
struct DemoConfig
{
    // ── חלון ──
    static constexpr const char* WINDOW_NAME = "Kung-Fu Chess Demo";

    // ── מימדי תא (פיקסלים) ──
    static constexpr int   CELL_SIZE       = 100;

    // ── לוח (שחמט תקני) ──
    static constexpr int   BOARD_COLS      = 8;
    static constexpr int   BOARD_ROWS      = 8;

    // ── מימדי חלון (נגזרים) ──
    static constexpr int   BOARD_WIDTH_PX  = CELL_SIZE * BOARD_COLS;   // 800
    static constexpr int   BOARD_HEIGHT_PX = CELL_SIZE * BOARD_ROWS;   // 800
    static constexpr int   PANEL_WIDTH_PX  = 150;
    static constexpr int   CANVAS_WIDTH_PX = BOARD_WIDTH_PX + PANEL_WIDTH_PX * 2;  // 1100

    // ── FPS ≈ 60 ──
    static constexpr int   FRAME_DELAY_MS  = 16;

    // ── אנימציות ──
    static constexpr int   MOVE_DURATION_MS = 500;
    static constexpr int   JUMP_HEIGHT_PX   = 40;

    // ── צבעים (BGR, טווח 0–255) ──
    // cv::Vec3b לא constexpr — לכן const רגיל.
    // בפועל הקומפיילר יודע לאופטם גם ככה.
    static inline const cv::Vec3b HIGHLIGHT  = { 80, 200, 120};  // גוון ירוק
    static inline const cv::Vec3b SELECTED   = { 50, 130, 255};  // גוון כחול
    static inline const cv::Vec3b TEXT_COLOR = {255, 255, 255};  // לבן

    // ── Game-over overlay ──
    static constexpr double OVERLAY_FONT_SCALE = 2.0;
    static constexpr int    OVERLAY_THICKNESS  = 3;
};
