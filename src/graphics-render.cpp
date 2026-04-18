#include<ppe.hpp>

namespace ppe {

    // img: BGR float
    void Graphics::_renderLayers(cv::Mat& img) const {
        cv::Mat az, eq;
        cv::Matx33f rot = __calcLoc(az, eq);

        az.release();
        eq.release();
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
    // return: rotation matrix (eq->xy)
    cv::Matx33f Graphics::__calcLoc(cv::Mat& az, cv::Mat& eq) const {
        az.create(_config.height, _config.width, CV_32FC2);
        eq.create(_config.height, _config.width, CV_32FC2);

        int pov = std::min(_config.height, _config.width);

        cv::Vec2f* paz = reinterpret_cast<cv::Vec2f*>(az.data);
        cv::Vec2f* peq = reinterpret_cast<cv::Vec2f*>(eq.data);

        cv::Matx33f xy2az = rotationZ(_status.azimuth) * rotationX(_status.elevation);
        cv::Matx33f az2eq = rotationZ(_status.longitude + (float)(_status.simTime * ROT_SIDEREAL)) * rotationX(_status.latitude);
        cv::Matx33f xy2eq = az2eq * xy2az;

        #pragma omp parallel for
        for(int i = 0; i < _imagePixels; i++) {
            int x = i % _config.width, y = i / _config.width;
            float fx = x / (float)(pov-1) - 0.5, fy = y / (float)(pov-1) - 0.5;

            float phi = std::atan2(fy, fx);
            float theta = 0.5 * M_PI - std::atan(std::sqrt(fx*fx+fy*fy) * std::tan(0.5 * _status.fov));
            cv::Vec3f vxy(
                std::cos(phi) * std::sin(theta),
                std::sin(phi) * std::sin(theta),
                std::cos(theta)
            );

            cv::Vec3f vaz = xy2az * vxy;
            paz[0] = std::atan2(vaz[1], vaz[0]);
            paz[1] = std::asin(vaz[2]);

            cv::Vec3f veq = az2eq * vaz;
            peq[0] = std::atan2(veq[1], veq[0]);
            peq[1] = std::asin(veq[2]);
        }

        return xy2eq.inv();
    }

}