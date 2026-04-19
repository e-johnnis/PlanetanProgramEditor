#ifndef __PPE_GRAPHICS_HPP
#define __PPE_GRAPHICS_HPP 0

#include<ppe/core.hpp>
#include<opencv2/opencv.hpp>
#include<vector>

#define PPE_DIFFRACTION_SIZE 11

namespace ppe {

    struct GraphicsConfig {
        char inputFileName[PPE_CHAR_MAX];
        char outputFileName[PPE_CHAR_MAX];
        int width = 1920;
        int height = 1080;
        double fps = 30;
    };

    struct SimulationStatus {
        unsigned long frames = 0;

        // observation
        double simTime = 0;             // days from J2000.0
        float latitude = 0;             // rad
        float longitude = 0;            // rad
        float elevation = 0;            // rad
        float azimuth = 0;              // rad
        float fov = 2.0 * M_PI / 3.0;   // rad

        // rendering levels (0-1)
        float lvStars = 0;
        float lvGround = 0;
        float lvGrowRed = 0;
        float lvGrowGreen = 0;      
        float lvGrowBlue = 0;
        float lvTwilightRed = 0;
        float lvTwilightGreen = 0;
        float lvTwilightBlue = 0;
    };

    class Graphics {
    public:
        Graphics(const GraphicsConfig*);
        ~Graphics();
        void release();
        int render(double*);

        int loadEvents(int*, int*);
        int addEvent(const char*);

        int loadStars();
        int loadConsts();

        double endTime = 0;
    private:
        void _renderLayers(cv::Mat&) const;
        void __overlay(cv::Mat&, const cv::Mat&) const;
        cv::Matx33f __calcLoc(cv::Mat&, cv::Mat&, cv::Mat&) const;
        void __calcStars(const cv::Matx33f&) const;

        GraphicsConfig _config = {};
        SimulationStatus _status = {};

        int _imagePixels = 0;

        cv::VideoWriter _videoWriter;
        cv::Mat _diffractionKernel;

        std::vector<Event*> _events;

        std::vector<Star> _stars;
        std::vector<ConstLine> _constLines;
    };
}

#endif