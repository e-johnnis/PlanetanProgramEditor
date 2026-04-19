#include<ppe.hpp>
#include<cstdio>
#include<cstring>
#include<cmath>

namespace ppe {

    Graphics::Graphics(const GraphicsConfig* config) {
        _config = *config;
        _imagePixels = _config.width * _config.height;

        _diffractionKernel.create(PPE_DIFFRACTION_SIZE, PPE_DIFFRACTION_SIZE, CV_32FC1);
        float* pdiff = reinterpret_cast<float*>(_diffractionKernel.data);
        for(int x = 0; x < PPE_DIFFRACTION_SIZE; x++) {
            double fx = 2.0 * (x / (double)(PPE_DIFFRACTION_SIZE-1) - 0.5);
            for(int y = 0; y < PPE_DIFFRACTION_SIZE; y++) {
                double fy = 2.0 * (y / (double)(PPE_DIFFRACTION_SIZE-1) - 0.5);
                pdiff[x+y*PPE_DIFFRACTION_SIZE] = (float)getLevel(1.0 - std::sqrt(fx * fx + fy * fy), CHANGE_SMOOTHER);
            }
        }

        #ifdef DEBUG
        {
            cv::Mat diffImg;
            _diffractionKernel.convertTo(diffImg, CV_8UC1, 255, 0);
            cv::imwrite("debug/diffraction.png", diffImg);
            diffImg.release();
        }
        #endif

        _videoWriter.open(
            _config.outputFileName, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
            _config.fps, cv::Size(_config.width, _config.height)
        );
    }

    Graphics::~Graphics() {
        release();
    }

    void Graphics::release() {
        _videoWriter.release();
        _diffractionKernel.release();

        for(int i = 0; i < _events.size(); i++) if(_events[i]) delete _events[i];
        _events.clear();
        _stars.clear();
        _constLines.clear();
    }

    int Graphics::render(double* frameTime) {
        *frameTime = _status.frames / _config.fps;
        if(*frameTime > endTime) return 0;

        // BGR float
        cv::Mat img = cv::Mat::zeros(_config.height, _config.width, CV_32FC3);

        // call events
        for(int i = 0; i < _events.size(); i++) if(_events[i] && _events[i]->call(*frameTime)) {
            delete _events[i];
            _events[i] = nullptr;
        }

        _renderLayers(img);

        cv::Mat frame;
        img.convertTo(frame, CV_8UC3, 255, 0);

        _videoWriter.write(frame);

        img.release();
        frame.release();

        _status.frames++;
        return 1;
    }

    int Graphics::loadEvents(int* nline, int* nevent) {
        FILE* fp = std::fopen(_config.inputFileName, "r");
        if(!fp) return ERROR_OPEN_FILE;

        *nline = 0;
        *nevent = 0;

        #ifdef DEBUG
            std::printf("\n");
        #endif

        while(!std::feof(fp)) {
            char line[PPE_CHAR_MAX] { '\0' };
            char cmd[PPE_CHAR_MAX] { '\0' };
            int err = SUCCESS;

            (*nline)++;
            
            std::fgets(line, PPE_CHAR_MAX-1, fp);
            if(!std::strlen(line) || (line[0] == '\n')) continue;
            if(!std::sscanf(line, "%[^#\n]", cmd)) continue;

            #ifdef DEBUG
                std::printf("line %3d: %s\n", *nline, cmd);
            #endif

            if((err = addEvent(cmd))) {
                std::fclose(fp);
                return err;
            }

            (*nevent)++;
        }
        std::fclose(fp);

        return SUCCESS;
    }

    int Graphics::addEvent(const char* cmd) {
        char arg[PPE_CHAR_MAX] { '\0' };
        char startstr[16] { '\0' };
        char endstr[16] { '\0' };
        char ename[16] { '\0' };
        double start = 0, end = 0;

        if(
            (std::sscanf(cmd, "%s %s %s %[^\n]", startstr, endstr, ename, arg) < 3)||
            !parseTime(startstr, &start) || !parseTime(endstr, &end)
        ) {
            return ERROR_INVALID_FORMAT;
        }

        if(!std::strcmp(ename, "video")) {
            endTime = end;
        }else if(!std::strcmp(ename, "time")) {
            double value = 0;
            if(!parseDateTime(arg, &value)) return ERROR_INVALID_VALUE;
            _events.push_back(new ChangeValueEvent<double>(start, end, &_status.simTime, value, CHANGE_LINEAR));
        
        }else if(!std::strcmp(ename, "latitude")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.latitude, radians(value), ctype));
        
        }else if(!strcmp(ename, "longitude")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.longitude, radians(value), ctype));
        
        }else if(!strcmp(ename, "elevation")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.elevation, radians(value), ctype));
        
        }else if(!strcmp(ename, "azimuth")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.azimuth, radians(value), ctype));
        
        }else if(!strcmp(ename, "fov")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.fov, radians(value), ctype));
        
        }else if(!strcmp(ename, "stars")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvStars, value, ctype));
        
        }else if(!strcmp(ename, "ground")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvGround, value, ctype));
        
        }else if(!strcmp(ename, "growRed")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvGrowRed, value, ctype));
        
        }else if(!strcmp(ename, "growGreen")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvGrowGreen, value, ctype));
        
        }else if(!strcmp(ename, "growBlue")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvGrowBlue, value, ctype));
        
        }else if(!strcmp(ename, "twilightRed")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvTwilightRed, value, ctype));
        
        }else if(!strcmp(ename, "twilightGreen")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvTwilightGreen, value, ctype));
        
        }else if(!strcmp(ename, "twilightBlue")) {
            char ctstr[16] { '\0' };
            float value = 0;
            int ctype = CHANGE_LINEAR;
            if(!std::sscanf(arg, "%f %s", &value, ctstr)) return ERROR_INVALID_VALUE;
            if(std::strlen(ctstr) && !parseChangeType(ctstr, &ctype)) return ERROR_UNKNOWN_ENUM;
            _events.push_back(new ChangeValueEvent<float>(start, end, &_status.lvTwilightBlue, value, ctype));
        
        }else {
            return ERROR_UNKNOWN_EVENT;
        }

        return SUCCESS;
    }

    int Graphics::loadStars() {
        char path[PPE_CHAR_MAX];
        getResourcePath(path, "stars.nsc");

        FILE* fp = std::fopen(path, "r");
        if(!fp) return ERROR_OPEN_FILE;

        cv::Matx33f eqj2ecl = rotationX(TILT_EARTH);

        while(!std::feof(fp)) {
            char line[PPE_CHAR_MAX] { '\0' };
            int catalogNum;
            float magnitude;
            double ra, dec;
            char spType;

            fgets(line, PPE_CHAR_MAX-1, fp);
            if(sscanf(
                line, "%d:%lf,%lf:%f:%c",
                &catalogNum, &ra, &dec, &magnitude, &spType
            ) < 5) continue;

            Star star;

            star.catalogNum = catalogNum;
            star.ecl = cv::Vec3f(
                std::cos(ra) * std::cos(dec),
                std::sin(ra) * std::cos(dec),
                std::sin(dec)
            );
            
            switch(spType) {
                case 'O':
                    star.color = cv::Vec3f(0x92/255.0, 0xB5/255.0, 0xFF/255.0);
                    break;
                case 'B':
                    star.color = cv::Vec3f(0xA2/255.0, 0xC0/255.0, 0xFF/255.0);
                    break;
                case 'A':
                    star.color = cv::Vec3f(0xD5/255.0, 0xE0/255.0, 0xFF/255.0);
                    break;
                case 'G':
                    star.color = cv::Vec3f(0xFF/255.0, 0xED/255.0, 0xE3/255.0);
                    break;
                case 'K':
                    star.color = cv::Vec3f(0xFF/255.0, 0xDA/255.0, 0xB5/255.0);
                    break;
                case 'M':
                    star.color = cv::Vec3f(0xFF/255.0, 0xB5/255.0, 0x6C/255.0);
                    break;
                default: // F
                    star.color = cv::Vec3f(0xF9/255.0, 0xF5/255.0, 0xFF/255.0);
                    break;
            }

            star.magnitude = magnitude;

            _stars.push_back(star);
        }

        std::fclose(fp);

        #ifdef DEBUG
            std::printf("\n");
            for(int i = (_stars.size() - 10); i < _stars.size(); i++) {
                std::printf(
                    "%d\t(%f, %f, %f)\t(%f, %f, %f)\t%f\n",
                    _stars[i].catalogNum,
                    _stars[i].ecl[0], _stars[i].ecl[1], _stars[i].ecl[2],
                    _stars[i].color[0], _stars[i].color[1], _stars[i].color[2],
                    _stars[i].magnitude
                );
            }
            std::printf("stars loaded: %lu\n", _stars.size());
        #endif

        return SUCCESS;
    }

    int Graphics::loadConsts() {
        char path[PPE_CHAR_MAX];
        getResourcePath(path, "const.pcl");

        FILE* fp = std::fopen(path, "r");
        if(!fp) return ERROR_OPEN_FILE;

        ConstLine constLine;
        int ns = 0;
        std::memset(&constLine, 0, sizeof(ConstLine));

        while(!std::feof(fp)) {
            char cnstr[16] { '\0' };
            int catalogNum = -1;
            if(!fscanf(fp, "%s", cnstr)) continue;

            if(sscanf(cnstr, "%d", &catalogNum)) {
                constLine.stars[ns] = catalogNum;
                ns++;
            }else if(std::strlen(cnstr)) {
                std::strcpy(constLine.name, cnstr);
                _constLines.push_back(constLine);
                std::memset(&constLine, 0, sizeof(ConstLine));
                ns = 0;
            }
        }

        std::fclose(fp);

        #ifdef DEBUG
            std::printf("\n");
            for(int i = (_constLines.size() - 10); i < _constLines.size(); i++) {
                std::printf("%s", _constLines[i].name);
                for(int j = 0; j < 64; j++) std::printf(" %d", _constLines[i].stars[j]);
                std::printf("\n");
            }
            std::printf("constLines loaded: %lu\n", _constLines.size());
        #endif

        return SUCCESS;
    }

}