//
// Created by lorre851 on 01.03.17.
//

#ifndef THESIS_CVSERVER_H
#define THESIS_CVSERVER_H

#include "opencv2/opencv.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <queue>
#include <semaphore.h>

using namespace std;
using namespace cv;


class cvserver {
    private:
        /*      Server parameters       */
        int port, localSocket, remoteSocket = EWOULDBLOCK;
        struct  sockaddr_in localAddr, remoteAddr;
        int addrLen = sizeof(struct sockaddr_in);

        /*        Mat objects         */
        Mat *image = new Mat();
        Mat no_image = Mat::zeros(360 , 640, CV_8UC1);
        //queue<Mat*> buffer;
	    Mat *buffer = NULL;

        /*        Threads & Flags      */
        thread *server = NULL;
        volatile bool ready = false;        //is server ready to accept connections? (was init() successful?)
        volatile bool lock_data = false;    //data lock flag for multithread access
        volatile bool debug = false;        //debug flag - enables debug window

        /*           Functions          */
        void lock();                //lock Mat data access
        void unlock();              //release Mat data access
        void loop_server();         //wait for clients & send them data

        struct timeval timeout;
    public:
        cvserver(int);
        ~cvserver();
        void init();
        void close_conn();
        bool toggle_debug();
        void reset();
        bool push_data(const Mat*);
        bool is_ready();
};


#endif //THESIS_CVSERVER_H
