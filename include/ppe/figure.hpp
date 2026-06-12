#ifndef __PPE_FIGURE_HPP
#define __PPE_FIGURE_HPP 0

#define PPE_FIGURE_TAG_MAX 32

#include<ppe/astrObject.hpp>

namespace ppe {

    class Figure {
    public:
        Figure(const char*);
        virtual ~Figure() {};

        int cmptag(const char*);

        // img: BGRA float
        virtual void draw(cv::Mat&) = 0;

        float level = 0;
        float progress = 0;
    protected:
        char _tag[PPE_FIGURE_TAG_MAX];
    };

    class Line : public Figure {
    public:
        Line(const char*, const Star*, const Star*);
        ~Line() override {};

        void draw(cv::Mat&) override;
    protected:
        const Star* _start = NULL;
        const Star* _end = NULL;
    };

    class Circle : public Figure {
    public:
        Circle(const char*, const Star*, int);
        ~Circle() override {};

        void draw(cv::Mat&) override;
    protected:
        const Star* _target = NULL;
        int _radius = -1;
    };
}

#endif