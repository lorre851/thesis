//
// Created by lorre851 on 03.12.16.
//

#ifndef THESIS_CAMERASCANNER_H
#define THESIS_CAMERASCANNER_H


#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <ostream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include "opencv2/opencv.hpp"


#define CAMERA_COUNT 5
#define ILLEGAL_CAM_ID -1
#define NOT_CONNECTED -2

#define FPS_SLOW 100000
#define FPS_MED 25000
#define FPS_FAST 5000

using namespace cv;
using namespace std;


struct CAMSCAN_camera_properties {
    string name;
    string identifier;
    int width;
    int height;

};

class camerascanner {
private:
    //atomic variables
    volatile bool cam_status[CAMERA_COUNT];
    volatile bool locked[CAMERA_COUNT];


    //variables
    int cooldown[CAMERA_COUNT];
    VideoCapture *camera[CAMERA_COUNT];
    CAMSCAN_camera_properties properties[CAMERA_COUNT];
    Mat *frames[CAMERA_COUNT];
    bool thread_scan_run = false;
    bool thread_grab_run = false;
    int lowest_cam = 0;
    int grabspeed = FPS_SLOW;
    thread *scanner = NULL;
    thread *grabber = NULL;

    //tcp feedback variables
    string tcpinternal;
    string *tcperrno = &tcpinternal;

    //functions
    int allocate(int, v4l2_capability &);
    int deallocate(int);
    bool is_valid(int);
    void tcpstatus(string);
    string u8_string(__u8*);



public:
    //constructor
    camerascanner(bool);

    //functions
    void scan(int);
    void grab_frames(int);
    Mat* frame(int);
    Mat** get_stereo_frame(int, int);
    thread *thread_scan();
    thread *thread_grab();
    bool thread_scan_join();
    bool thread_grab_join();
    int force_refresh(int);
    string identifier(int);
    void set_grab_speed(int);
    void set_tcperrno(string*);



};


#endif //THESIS_CAMERASCANNER_H