#pragma once

/// @file img.hpp
/// @brief מעטפת על OpenCV — מורחבת מתוך ה-repo של KamaTech.
///
/// המקור מ- https://github.com/KamaTechOrg/CTD26/tree/main/cpp/src/img.hpp
/// הורחב במתודות הנדרשות לפרויקט הדמו:
///   - create, resize_to, fill_rect, fill_circle, blend_overlay
///   - named_window, show_window, wait_key, set_mouse_callback
///   - destroy_window, get_window_property
///   - clone

#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>
#include <utility>

class Img {
public:
    Img();

    // ═══════════════════════════════════════════════════
    //  מתודות מקוריות מ-KamaTech
    // ═══════════════════════════════════════════════════

    /// טוען תמונה מקובץ, resize אופציונלי
    Img& read(const std::string& path,
              const std::pair<int, int>& size = {},
              bool keep_aspect = false,
              int interpolation = cv::INTER_AREA);

    /// מצייר Img זה על גבי Img אחר (כולל מיזוג אלפא)
    void draw_on(Img& other_img, int x, int y) const;

    /// כותב טקסט על התמונה
    void put_text(const std::string& txt, int x, int y, double font_size,
                  const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                  int thickness = 1);

    /// מציג בחלון (חוסם — waitKey(0))
    void show();

    const cv::Mat& get_mat() const { return img; }
    cv::Mat& get_mat() { return img; }
    bool is_loaded() const { return !img.empty(); }

    // ═══════════════════════════════════════════════════
    //  מתודות מורחבות — ציור וצורות
    // ═══════════════════════════════════════════════════

    /// יוצר Img ריק בגודל נתון
    static Img create(int width, int height, int channels = 3);

    /// שינוי גודל (in-place)
    Img& resize_to(int width, int height, int interpolation = cv::INTER_LINEAR);

    /// ציור מלבן מלא
    Img& fill_rect(int x, int y, int w, int h, const cv::Scalar& color);

    /// ציור עיגול (thickness = cv::FILLED כברירת מחדל)
    Img& fill_circle(int cx, int cy, int radius, const cv::Scalar& color,
                     int thickness = cv::FILLED);

    /// מיזוג חצי-שקוף: this = this*(1-alpha) + overlay*alpha
    Img& blend_overlay(const Img& overlay, double alpha);

    /// שכפול עמוק
    Img clone() const;

    // ═══════════════════════════════════════════════════
    //  מתודות מורחבות — ניהול חלון
    // ═══════════════════════════════════════════════════

    static void named_window(const std::string& winname,
                             int flags = cv::WINDOW_NORMAL);
    static void show_window(const std::string& winname, const Img& img);
    static int  wait_key(int delayMs);
    static void set_mouse_callback(const std::string& winname,
                                   cv::MouseCallback onMouse,
                                   void* userdata);
    static void destroy_window(const std::string& winname);
    static void destroy_all_windows();
    static int  get_window_property(const std::string& winname, int propId);

    // ═══════════════════════════════════════════════════
    //  גישה למימדים
    // ═══════════════════════════════════════════════════

    int width()  const { return img.cols; }
    int height() const { return img.rows; }
    int channels() const { return img.channels(); }

private:
    cv::Mat img;
};
