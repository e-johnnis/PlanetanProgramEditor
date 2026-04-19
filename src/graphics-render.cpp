#include<ppe.hpp>

namespace ppe {

    // img: BGR float
    void Graphics::_renderLayers(cv::Mat& img) {
        cv::Mat az, eq, ecl;
        cv::Matx33f rot = __calcLoc(az, eq, ecl);
        __calcStars(rot);

        az.release();
        eq.release();
        ecl.release();
    }


    // base: BGR float
    // top: BGRA float
    void Graphics::__overlay(cv::Mat& base, const cv::Mat& top) const {
        cv::Vec3f* pbase = reinterpret_cast<cv::Vec3f*>(base.data);
        const cv::Vec4f* ptop = reinterpret_cast<const cv::Vec4f*>(top.data);

        #pragma omp parallel for
        for(int i = 0; i < _imagePixels; i++) {
            for(int j = 0; j < 3; j++) pbase[i][j] = pbase[i][j] * (1.0 - ptop[i][3]) + ptop[i][j] * ptop[i][3];
        }
    }

    // az: az, el
    // eq: ra, dec
    // ecl: bet, lam
    // return: rotation matrix (ecl->xy)
    cv::Matx33f Graphics::__calcLoc(cv::Mat& az, cv::Mat& eq, cv::Mat& ecl) const {
        az.create(_config.height, _config.width, CV_32FC2);
        eq.create(_config.height, _config.width, CV_32FC2);
        ecl.create(_config.height, _config.width, CV_32FC2);

        int pov = std::min(_config.height, _config.width);
        float ph = std::tan(0.25 * _status.fov);

        cv::Vec2f* paz = reinterpret_cast<cv::Vec2f*>(az.data);
        cv::Vec2f* peq = reinterpret_cast<cv::Vec2f*>(eq.data);
        cv::Vec2f* pecl = reinterpret_cast<cv::Vec2f*>(ecl.data);

        cv::Matx33f xy2az = rotationZ(_status.azimuth) * rotationX(_status.elevation);
        cv::Matx33f az2eq = rotationZ(_status.longitude + (float)(_status.simTime * ROT_SIDEREAL)) * rotationX(_status.latitude - 0.5 * M_PI);
        cv::Matx33f eq2ecl = rotationZ((float)(-ROT_PRECESSION * _status.simTime)) * rotationX(-TILT_EARTH);
        cv::Matx33f xy2ecl = eq2ecl * az2eq * xy2az;

        #pragma omp parallel for
        for(int i = 0; i < _imagePixels; i++) {
            int x = i % _config.width, y = i / _config.width;
            float fx = x / (float)(pov-1) - 0.5, fy = y / (float)(pov-1) - 0.5;

            float phi = std::atan2(fy, fx);
            float theta = 0.5 * M_PI - std::atan(std::sqrt(fx*fx+fy*fy) * ph);
            cv::Vec3f vxy(
                std::cos(phi) * std::sin(theta),
                std::sin(phi) * std::sin(theta),
                std::cos(theta)
            );

            cv::Vec3f vaz = xy2az * vxy;
            paz[i][0] = std::atan2(vaz[1], vaz[0]);
            paz[i][1] = std::acos(vaz[2]);

            cv::Vec3f veq = az2eq * vaz;
            peq[i][0] = std::atan2(veq[1], veq[0]);
            peq[i][1] = std::acos(veq[2]);

            cv::Vec3f vecl = eq2ecl * veq;
            pecl[i][0] = std::atan2(vecl[1], vecl[0]);
            pecl[i][1] = std::acos(vecl[2]);
        }

        return xy2ecl.inv();
    }

    void Graphics::__calcStars(const cv::Matx33f& ecl2xy) {
        int pov = std::min(_config.height, _config.width);
        float ph = 2.0 / std::tan(0.25 * _status.fov);

        #pragma omp parallel for
        for(int i = 0; i < _stars.size(); i++) {
            cv::Vec3f vxy = ecl2xy * _stars[i].ecl;
            float phi = std::atan2(vxy[1], vxy[0]), theta = std::acos(vxy[2]);
            float tt = std::tan(theta);

            _stars[i].x = pov * (ph * std::cos(phi) * tt + 0.5);
            _stars[i].y = pov * (ph * std::sin(phi) * tt + 0.5);
        }
    }

    ConstLine* Graphics::__findConst(const char* name) const {
        for(int i = 0; i < _constLines.size(); i++) {
            if(!std::strcmp(_constLines[i].name, name)) return &_constLines[i];
        }
        return nullptr;
    }

}