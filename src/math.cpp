#include<ppe.hpp>
#include<cmath>

namespace ppe {

    float radians(float a) { return M_PI * a / 180.0; }
    float degrees(float a) { return 180.0 * a / M_PI; }

    cv::Matx33f rotationX(float a) {
        return cv::Matx33f(
            1,              0,              0,
            0,              std::cos(a),    -std::sin(a),
            0,              std::sin(a),    std::cos(a)
        );
    }

    cv::Matx33f rotationY(float a) {
        return cv::Matx33f(
            std::cos(a),    0,              std::sin(a),
            0,              1,              0,
            -std::sin(a),   0,              std::cos(a)
        );
    }

    cv::Matx33f rotationZ(float a) {
        return cv::Matx33f(
            std::cos(a),    -std::sin(a),   0,
            std::sin(a),    std::cos(a),    0,
            0,              0,              1
        );
    }

}