//
// Created by lorre851 on 31.03.17.
//

#ifndef CAMSERVER_DEPTHPEOPLEDETECTOR_H
#define CAMSERVER_DEPTHPEOPLEDETECTOR_H

#include <iostream>
#include <time.h>
#include "opencv2/opencv.hpp"

#define DIRECTION_UNKNOWN 0
#define DIRECTION_UP 1
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 3
#define DIRECTION_RIGHT 4

struct person {
    cv::Point2f min = cv::Point2f(-1, -1), max = cv::Point2f(-1, -1);
    cv::Point2f hist_1, hist_2, hist_3, estimated;
    int direction = DIRECTION_UNKNOWN;
    int misses = 0, tracked_for = 0;
    cv::KeyPoint last_keypoint;
    cv::Scalar color;
    int traversed_out = 0, traversed_in = 0;

};


class depthpeopledetector {
private:
    int MISS_THRESH = 5;
    int TRACK_REGION = 25;
    int BORDER_SIZE = 40;

    float trans_thresh = 0.18;

    int traversed_out = 0, traversed_in = 0;
    time_t seconds = time(NULL);
    cv::RNG rng;
    cv::Rect img_roi;
    std::vector<struct person> tracker;
    void aggergatekeypoints(std::vector<cv::KeyPoint> &);
    bool eliminate_overlap(struct person &, struct person &);
    void estimate_next(struct person &);
    void push_point_history(struct person &p);

    bool dev = false;
    bool visual = false;
public:
    depthpeopledetector();
    void detect(cv::Mat &);
    void update_trackers();
    void devel(bool);
    void visualize(bool);
    cv::Size border_size;
    void cleanup();
    int results_in();
    int results_out();
};


#endif //CAMSERVER_DEPTHPEOPLEDETECTOR_H
