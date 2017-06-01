//
// Created by lorre851 on 22.02.17.
//

#ifndef THESIS_STREAMGRID_H
#define THESIS_STREAMGRID_H

#include "opencv2/opencv.hpp"


using namespace std;
using namespace cv;

class streamgrid {
    private:
        double width, height, grid_width ,grid_height;
        Mat* output;
        bool *occupied;
        void clear_occupied();
        string name;
        string default_name = "Streamgrid";
    public:
        streamgrid(int, int, int, int, string);
        streamgrid(int, int, int, int);
        ~streamgrid();
        int set_block(Mat&, int, string);
        int set_block(Mat&, int);
        void draw();
        void clear_all();

};


#endif //THESIS_STREAMGRID_H
