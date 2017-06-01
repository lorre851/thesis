//
// Created by lorre851 on 22.02.17.
//

#ifndef THESIS_STEREOTOOLS_H
#define THESIS_STEREOTOOLS_H

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"

#include <stdio.h>

using namespace std;
using namespace cv;

class stereotools {
    private:
        FileStorage *fs;
        Mat M1, D1, M2, D2, R, T;
        float scale = 1.0;
        bool init_rectify_map = false;
        Size img_size;
        Rect roi1, roi2;
        Mat Q;
        Mat R1, P1, R2, P2;
        Mat map11, map12, map21, map22;
        bool has_config = false;
	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0,16,4);
	int SADWindowSize, numberOfDisparities;
	int cn, sgbmWinSize;
    public:
        stereotools();
        int load_config(string, string);
        Mat* generate_depth_map(const Mat&, const Mat&);
        bool validate_config();
};


#endif //THESIS_STEREOTOOLS_H
