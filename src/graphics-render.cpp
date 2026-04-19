#include<ppe.hpp>

namespace ppe {

    // img: BGR float
    void Graphics::_renderLayers(cv::Mat& img) {
        cv::Mat az, eq, ecl;
        cv::Matx33f rot = __calcLoc(az, eq, ecl);
        __calcStars(rot);

        __renderStars(img);
        __renderConstLines(img);
        __renderGround(img, az);

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

        float ph = std::tan(0.25 * _status.fov);
        float hw = 0.5 * (_config.width-1), hh = 0.5 * (_config.height-1);
        float pov = std::min(hw, hh);

        cv::Vec2f* paz = reinterpret_cast<cv::Vec2f*>(az.data);
        cv::Vec2f* peq = reinterpret_cast<cv::Vec2f*>(eq.data);
        cv::Vec2f* pecl = reinterpret_cast<cv::Vec2f*>(ecl.data);

        cv::Matx33f xy2az = rotationZ(_status.azimuth) * rotationX(_status.elevation - 0.5 * M_PI);
        cv::Matx33f az2eq = rotationZ(_status.longitude + (float)(_status.simTime * ROT_SIDEREAL)) * rotationX(_status.latitude - 0.5 * M_PI);
        cv::Matx33f eq2ecl = rotationZ((float)(-ROT_PRECESSION * _status.simTime)) * rotationX(-TILT_EARTH);
        cv::Matx33f xy2ecl = eq2ecl * az2eq * xy2az;

        #pragma omp parallel for
        for(int i = 0; i < _imagePixels; i++) {
            int x = i % _config.width, y = i / _config.width;
            float fx = (x - hw) / pov, fy = (y - hh) / pov;

            float phi = std::atan2(fy, fx);
            float theta = 2.0 * std::atan(std::sqrt(fx*fx+fy*fy) * ph);
            cv::Vec3f vxy(
                std::cos(phi) * std::sin(theta),
                std::sin(phi) * std::sin(theta),
                std::cos(theta)
            );

            cv::Vec3f vaz = xy2az * vxy;
            paz[i][0] = std::atan2(vaz[1], vaz[0]);
            paz[i][1] = std::asin(vaz[2]);

            cv::Vec3f veq = az2eq * vaz;
            peq[i][0] = std::atan2(veq[1], veq[0]);
            peq[i][1] = std::asin(veq[2]);

            cv::Vec3f vecl = eq2ecl * veq;
            pecl[i][0] = std::atan2(vecl[1], vecl[0]);
            pecl[i][1] = std::asin(vecl[2]);
        }

        return xy2ecl.inv();
    }

    void Graphics::__calcStars(const cv::Matx33f& ecl2xy) {
        float ph = 0.5 * std::min(_config.height, _config.width) / std::tan(0.25 * _status.fov);
        float hw = 0.5 * (_config.width-1), hh = 0.5 * (_config.height-1);

        #pragma omp parallel for
        for(int i = 0; i < _stars.size(); i++) {
            cv::Vec3f vxy = ecl2xy * _stars[i].ecl;
            float phi = std::atan2(vxy[1], vxy[0]), theta = std::acos(vxy[2]);
            float tt = std::tan(theta * 0.5);

            _stars[i].x = ph * std::cos(phi) * tt + hw;
            _stars[i].y = ph * std::sin(phi) * tt + hh;
        }
    }

    ConstLine* Graphics::__findConst(const char* name){
        for(int i = 0; i < _constLines.size(); i++) {
            if(!std::strcmp(_constLines[i].name, name)) return &_constLines[i];
        }
        return nullptr;
    }

    // img: BGR float
    void Graphics::__renderStars(cv::Mat& img) const {
        if(_status.lvStars <= 0) return;

        cv::Mat mcpy = img.clone();
        cv::Vec3f* pimg = reinterpret_cast<cv::Vec3f*>(mcpy.data);

        for(int i = 0; i < _stars.size(); i++) {
            int x = _stars[i].x, y = _stars[i].y;
            if((x >= 0) && (x < _config.width) && (y >= 0) && (y < _config.height)) {
                int ipx = x + y * _config.width;
                float lmag = std::pow(10.0f, (4.0f - _stars[i].magnitude) / 4.0f) * _status.lvStars;
                for(int j = 0; j < 3; j++) pimg[ipx][j] += _stars[i].color[j] * lmag;
            }
        }

        img.release();
        cv::filter2D(mcpy, img, -1, _diffractionKernel);
    }

    // img: BGR float
    void Graphics::__renderConstLines(cv::Mat& img) const {
        cv::Mat layer = cv::Mat::zeros(img.size(), CV_32FC4);
        
        for(int i = 0; i < _constLines.size(); i++) {
            float lv = _constLines[i].level;
            if(lv <= 0) continue;
            cv::Scalar clcol(1.0, 0.8, 0.4, lv);

            for(int j = 0; j < PPE_CONSTLINE_STARS_MAX-1; j++) {
                int sa = _constLines[i].starIndices[j], sb = _constLines[i].starIndices[j+1];
                if((sa < 0) || (sb < 0)) continue;
                int ax = _stars[sa].x, ay = _stars[sa].y, bx = _stars[sb].x, by = _stars[sb].y;

                if(
                    ((ax >= 0) && (ax < _config.width) && (ay >= 0) && (ay < _config.height)) ||
                    ((bx >= 0) && (bx < _config.width) && (by >= 0) && (by < _config.height))
                ) {
                    cv::line(layer, cv::Point(ax, ay), cv::Point(bx, by), clcol, 1);
                }
            }
        }

        __overlay(img, layer);
        layer.release();
    }

    // img: BGR float
    // az : az, el
    void Graphics::__renderGround(cv::Mat& img, const cv::Mat& az) const {
        if(_status.lvGround <= 0) return;

        cv::Vec3f* pimg = reinterpret_cast<cv::Vec3f*>(img.data);
        const cv::Vec2f* paz = reinterpret_cast<const cv::Vec2f*>(az.data);
        float lug = 1.0 - _status.lvGround;

        #pragma omp parallel for
        for(int i = 0; i < _imagePixels; i++) if(paz[i][1] <= 0) {
            for(int j = 0; j < 3; j++) pimg[i][j] *= lug;
        }
    }

}