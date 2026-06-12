#include<ppe.hpp>
#include<opencv2/opencv.hpp>
#include<cstring>

namespace ppe {

    Figure::Figure(const char* tag) {
        std::strcpy(_tag, tag);
    }

    int Figure::cmptag(const char* tag) {
        return std::strcmp(_tag, tag);
    }
    
    Line::Line(const char* tag, const Star* start, const Star* end) : Figure(tag) {
        _start = start;
        _end = end;
    }

    void Line::draw(cv::Mat& img) {
        if(level <= 0 || progress <= 0) return;
        cv::Scalar clcol(1.0, 0.8, 0.4, level);

        cv::Vec2f vs(_start->x, _start->y);
        cv::Vec2f ve(_end->x, _end->y);
        cv::Vec2f vp = (1.0 - progress) * vs + progress * ve;

        cv::line(img, cv::Point((int)vs[0], (int)vs[1]), cv::Point((int)vp[0], (int)vp[1]), clcol, 1);
    }

    Circle::Circle(const char* tag, const Star* target, int radius) : Figure(tag) {
        _target = target;
        _radius = radius;
    }

    void Circle::draw(cv::Mat& img) {
        if(level <= 0 || progress <= 0) return;
        cv::Scalar cccol(0.4, 0.8, 1.0, level);

        cv::ellipse(img, cv::Point(_target->x, _target->y), cv::Size(_radius, _radius), 270, 0, 360 * progress, cccol, 1);
    }
};