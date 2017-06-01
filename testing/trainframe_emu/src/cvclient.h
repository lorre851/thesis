//
// Created by lorre851 on 10.03.17.
//

#ifndef CAMSERVER_CVCLIENT_H
#define CAMSERVER_CVCLIENT_H


#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <errno.h>
#include "opencv2/opencv.hpp"


class cvclient {
    private:
        int sokt;
        char serverIP[100];
        int serverPort;
        struct sockaddr_in serverAddr;
        socklen_t addrLen = sizeof(struct sockaddr_in);

        volatile bool ready = false;
        volatile bool lock_data = false;

        std::thread *client = NULL;

        void lock();
        void unlock();
        void loop_client();


        //std::queue<cv::Mat*> buffer;
        cv::Mat *framebuffer = NULL;
        cv::Mat no_image = cv::Mat::zeros(360 , 640, CV_8UC1);
    public:
        cvclient(int, std::string);
        bool is_ready();
        cv::Mat* get_data();
        void disconnect();
		static bool mat_is_valid(cv::Mat *);
};


#endif //CAMSERVER_CVCLIENT_H
