//
// Created by lorre851 on 22.02.17.
//

#ifndef THESIS_STREAMGRID_H
#define THESIS_STREAMGRID_H

#include "opencv2/opencv.hpp"




class streamgrid {
    private:
        double width, height, grid_width ,grid_height;
        cv::Mat* output;
        bool *occupied;
        void clear_occupied();
        std::string name;
        std::string default_name = "Streamgrid";
    public:
        streamgrid(int, int, int, int, std::string);
        streamgrid(int, int, int, int);
        ~streamgrid();
        int set_block(cv::Mat&, int, std::string);
        int set_block(cv::Mat&, int);
        void draw();
        void clear_all();

};


#endif //THESIS_STREAMGRID_H
