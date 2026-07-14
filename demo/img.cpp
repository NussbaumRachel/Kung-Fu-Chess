#include "img.hpp"
#include <iostream>
#include <stdexcept>

// ════════════════════════════════════════════════════════════════
//  Constructor
// ════════════════════════════════════════════════════════════════

Img::Img() {}

// ════════════════════════════════════════════════════════════════
//  read — טעינה מקובץ + resize אופציונלי (מקורי)
// ════════════════════════════════════════════════════════════════

Img& Img::read(const std::string& path,
               const std::pair<int, int>& size,
               bool keep_aspect,
               int interpolation)
{
    img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty())
        throw std::runtime_error("Cannot load image: " + path);

    if (size.first != 0 && size.second != 0)
    {
        int target_w = size.first;
        int target_h = size.second;
        int h = img.rows;
        int w = img.cols;

        if (keep_aspect)
        {
            double scale = std::min(static_cast<double>(target_w) / w,
                                    static_cast<double>(target_h) / h);
            int new_w = static_cast<int>(w * scale);
            int new_h = static_cast<int>(h * scale);
            cv::resize(img, img, cv::Size(new_w, new_h), 0, 0, interpolation);
        }
        else
        {
            cv::resize(img, img, cv::Size(target_w, target_h), 0, 0, interpolation);
        }
    }
    return *this;
}

// ════════════════════════════════════════════════════════════════
//  draw_on — ציור Img זה על Img אחר (מקורי, מורחב מעט)
// ════════════════════════════════════════════════════════════════

void Img::draw_on(Img& other_img, int x, int y) const
{
    if (img.empty() || other_img.img.empty())
        throw std::runtime_error("Both images must be loaded before drawing.");

    cv::Mat source_img = img;
    cv::Mat target_img = other_img.img;

    // התאמת ערוצים
    if (source_img.channels() != target_img.channels())
    {
        if (source_img.channels() == 3 && target_img.channels() == 4)
            cv::cvtColor(source_img, source_img, cv::COLOR_BGR2BGRA);
        else if (source_img.channels() == 4 && target_img.channels() == 3)
            cv::cvtColor(source_img, source_img, cv::COLOR_BGRA2BGR);
    }

    int h = source_img.rows;
    int w = source_img.cols;
    int H = target_img.rows;
    int W = target_img.cols;

    if (y + h > H || x + w > W)
        throw std::runtime_error("Image does not fit at the specified position.");

    cv::Mat roi = target_img(cv::Rect(x, y, w, h));

    if (source_img.channels() == 4)
    {
        std::vector<cv::Mat> channels;
        cv::split(source_img, channels);
        cv::Mat alpha;
        channels[3].convertTo(alpha, CV_32F, 1.0 / 255.0);

        for (int c = 0; c < 3; ++c)
        {
            roi.col(c).convertTo(roi.col(c), CV_32F);
            channels[c].convertTo(channels[c], CV_32F);
            // BGRA → BGR blending
            roi.col(c) = channels[c].mul(alpha) + roi.col(c).mul(1.0 - alpha);
            roi.col(c).convertTo(roi.col(c), CV_8U);
        }
    }
    else
    {
        source_img.copyTo(roi);
    }
}

// ════════════════════════════════════════════════════════════════
//  put_text (מקורי)
// ════════════════════════════════════════════════════════════════

void Img::put_text(const std::string& txt, int x, int y, double font_size,
                   const cv::Scalar& color, int thickness)
{
    if (img.empty())
        throw std::runtime_error("Image not loaded.");

    cv::putText(img, txt, cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX, font_size,
                color, thickness, cv::LINE_AA);
}

// ════════════════════════════════════════════════════════════════
//  show (מקורי — חוסם)
// ════════════════════════════════════════════════════════════════

void Img::show()
{
    if (img.empty())
        throw std::runtime_error("Image not loaded.");

    cv::imshow("Image", img);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

// ════════════════════════════════════════════════════════════════
//  create — יוצר Img בגודל נתון
// ════════════════════════════════════════════════════════════════

Img Img::create(int width, int height, int channels)
{
    Img result;
    int type = (channels == 4) ? CV_8UC4 : CV_8UC3;
    result.img = cv::Mat(height, width, type, cv::Scalar(0, 0, 0, 0));
    return result;
}

// ════════════════════════════════════════════════════════════════
//  resize_to
// ════════════════════════════════════════════════════════════════

Img& Img::resize_to(int width, int height, int interpolation)
{
    if (img.empty()) return *this;
    cv::resize(img, img, cv::Size(width, height), 0, 0, interpolation);
    return *this;
}

// ════════════════════════════════════════════════════════════════
//  fill_rect
// ════════════════════════════════════════════════════════════════

Img& Img::fill_rect(int x, int y, int w, int h, const cv::Scalar& color)
{
    if (img.empty()) return *this;
    cv::rectangle(img, cv::Rect(x, y, w, h), color, cv::FILLED);
    return *this;
}

// ════════════════════════════════════════════════════════════════
//  fill_circle
// ════════════════════════════════════════════════════════════════

Img& Img::fill_circle(int cx, int cy, int radius, const cv::Scalar& color, int thickness)
{
    if (img.empty()) return *this;
    cv::circle(img, cv::Point(cx, cy), radius, color, thickness);
    return *this;
}

// ════════════════════════════════════════════════════════════════
//  blend_overlay — מיזוג חצי-שקוף
// ════════════════════════════════════════════════════════════════

Img& Img::blend_overlay(const Img& overlay, double alpha)
{
    if (img.empty() || overlay.img.empty()) return *this;
    cv::addWeighted(overlay.img, alpha, img, 1.0 - alpha, 0, img);
    return *this;
}

// ════════════════════════════════════════════════════════════════
//  clone
// ════════════════════════════════════════════════════════════════

Img Img::clone() const
{
    Img result;
    if (!img.empty())
        result.img = img.clone();
    return result;
}

// ════════════════════════════════════════════════════════════════
//  ניהול חלון — מתודות סטטיות (עוטפות OpenCV HighGUI)
// ════════════════════════════════════════════════════════════════

void Img::named_window(const std::string& winname, int flags)
{
    cv::namedWindow(winname, flags);
}

void Img::show_window(const std::string& winname, const Img& img)
{
    cv::imshow(winname, img.img);
}

int Img::wait_key(int delayMs)
{
    return cv::waitKey(delayMs);
}

void Img::set_mouse_callback(const std::string& winname,
                             cv::MouseCallback onMouse,
                             void* userdata)
{
    cv::setMouseCallback(winname, onMouse, userdata);
}

void Img::destroy_window(const std::string& winname)
{
    cv::destroyWindow(winname);
}

void Img::destroy_all_windows()
{
    cv::destroyAllWindows();
}

int Img::get_window_property(const std::string& winname, int propId)
{
    return static_cast<int>(cv::getWindowProperty(winname, propId));
}
