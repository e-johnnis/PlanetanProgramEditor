#ifndef __PPE_MATH_HPP
#define __PPE_MATH_HPP 0

#include<opencv2/opencv.hpp>

// sidereal rotation [rad/day]
#define ROT_SIDEREAL 6.30038806468

// precession rotation [rad/day]
#define ROT_PRECESSION 6.67496015016e-7

// axis tilt of the earth [rad]
#define TILT_EARTH 0.409279709593

namespace ppe {

    float radians(float);
    float degrees(float);

    cv::Matx33f rotationX(float);
    cv::Matx33f rotationY(float);
    cv::Matx33f rotationZ(float);
}

#endif