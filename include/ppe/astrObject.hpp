#ifndef __PPE_ASTROBJECT_HPP
#define __PPE_ASTROBJECT_HPP 0

#include<opencv2/opencv.hpp>

namespace ppe {

    struct Star {
        int catalogNum;
        cv::Vec3f ecl;
        cv::Vec3f color;    // BGR
        float magnitude;
        int x;
        int y;
    };

    struct ConstLine {
        char name[4];
        int stars[64];
        float level = 0;
    };

}

#endif