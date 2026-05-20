#ifndef __PPE_ASTROBJECT_HPP
#define __PPE_ASTROBJECT_HPP 0

#include<opencv2/opencv.hpp>

#define PPE_STAR_NAME_MAX 16
#define PPE_CONSTLINE_STARS_MAX 64

namespace ppe {

    struct Star {
        int catalogNum;
        cv::Vec3f ecl;
        cv::Vec3f color;    // BGR
        float magnitude;
        int x = -1;
        int y = -1;
        cv::Vec3f levels;
        char name[PPE_STAR_NAME_MAX];
    };

    struct ConstLine {
        char name[4];
        int starIndices[PPE_CONSTLINE_STARS_MAX];
        float level = 0;
    };

}

#endif