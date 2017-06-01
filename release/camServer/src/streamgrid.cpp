//
// Created by lorre851 on 22.02.17.
//

#include "streamgrid.h"

/*
 *      STREAMGRID
 *      width / height: number of streamed images in the grid
 *      grid_width / grid_height: total size of the grid in pixels
 *
 */

void streamgrid::clear_occupied() {
    for(int i = 0; i < width * height; i++) occupied[i] = false;
}


streamgrid::streamgrid(int width, int height, int grid_width, int grid_height, std::string name) {
    if(width <= 0) width = 1;
    if(height <= 0) height = 1;
    if(grid_width <= 150) grid_width = 150;
    if(grid_height <= 150) grid_height = 150;
    this -> width = width;
    this -> height = height;
    this -> grid_width = grid_width;
    this -> grid_height = grid_height;
    if(name != "") this -> name = name;
    else this -> name = default_name;

    output = new cv::Mat(grid_height, grid_width, CV_8UC3);

    occupied = new bool[width * height];
    clear_occupied();
    //namedWindow(name, 0);
}



streamgrid::~streamgrid() {
    delete output;
    delete occupied;
    cv::destroyWindow(name);
}


int streamgrid::set_block(cv::Mat &source, int index) {
    set_block(source, index, "");
}

int streamgrid::set_block(cv::Mat &source, int index, std::string text) {
    int gwidth = index, gheight = 0;
    while(gwidth >= width) {
        gwidth -= width;
        gheight++;
    }
    int xwidth = grid_width / width;
    int ywidth = grid_height / height;
    int xstart = xwidth * gwidth;
    int ystart = ywidth * gheight;


    if(index < 0 || index > (height * width)) {
        std::cout << "[streamgrid] <set_block> ERROR: incorrect index ID" << std::endl;
        return 1;
    }
    else if(&source == NULL) {
        putText(*output, "ERROR 3 - NULL", cv::Point(xstart + 10, ystart + 75), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
        return 3;
    }
    else if(CV_8UC3 != source.type()) {
        putText(*output, "ERROR 2 - TYPE", cv::Point(xstart + 10, ystart + 75), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
        return 2;
    }
    else {

        cv::Mat *temp = new cv::Mat();
        resize(source, *temp,  cv::Size(xwidth, ywidth));

        if(xstart + xwidth > grid_width) xwidth = grid_width - xstart - 1;          // 1 uur zoeken. >= moest > zijn.
        if(ystart + ywidth > grid_height) ywidth = grid_height - ystart - 1;        // 22/2/2017 never forget


        temp -> copyTo((*output)(cv::Rect(xstart, ystart, xwidth, ywidth)));

        if(text != "") putText(*output, text, cv::Point(xstart + 10, ystart + 75), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
        //fml.
        occupied[index] = true;
        delete temp;
        return 0;
    }
}

void streamgrid::draw() {
    cv::Mat *prep = new cv::Mat(*output);
    int xwidth = grid_width / width;
    int ywidth = grid_height / height;

    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            rectangle(*prep, cv::Point(x * xwidth, y * ywidth), cv::Point(x * xwidth + xwidth, y * ywidth + ywidth), cv::Scalar(255, 255, 255));
            putText(*prep, std::to_string((int)(x + width * y)), cv::Point(x * xwidth + 10, y * ywidth + 50), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 0, 0), 2);
            if(!occupied[(int)(x + width * y)]) putText(*output, "*** no stream ***", cv::Point(x * xwidth + 60, y * ywidth + 50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
    }

    cv::imshow(name, *prep);
    cv::waitKey(1);
    clear_occupied();
    delete prep;
}

void streamgrid::clear_all() {
    delete output;
    output = new cv::Mat(grid_width, grid_height, CV_8UC3);
}

