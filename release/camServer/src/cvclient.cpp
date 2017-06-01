//
// Created by lorre851 on 10.03.17.
//

#include "cvclient.h"


cvclient::cvclient(int port, std::string ip) {
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    //std::cout << "constructing..." << std::endl;
    serverPort = port;
    strcpy(serverIP, ip.c_str());


    if ((sokt = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket() failed! " << serverIP << ":" << port << std::endl;

    }
    else {
        serverAddr.sin_family = PF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(serverIP);
        serverAddr.sin_port = htons(serverPort);

        if (setsockopt(sokt, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            std::cout << "[cvserver] <init> WARN: failed to set socket options! [0x02]" << std::endl;
        }
        if (setsockopt(sokt, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            std::cout << "[cvserver] <init> WARN: failed to set socket options! [0x03]" << std::endl;
        }

        if (connect(sokt, (sockaddr*)&serverAddr, addrLen) < 0) {
            std::cerr << "connect() failed! "<< serverIP << ":" << serverPort << std::endl;
        }
        else {
            client = new std::thread(&cvclient::loop_client, this);
            ready = true;
            is_connected = true;
        }
    }
    //std::cout << "constructing done." << std::endl;
}


cvclient::~cvclient() {
    if(framebuffer != NULL) delete framebuffer;
    if(client != NULL) delete client;

}

void cvclient::lock() {
    while(lock_data);
    lock_data = true;
}

void cvclient::unlock() {
    lock_data = false;
}

void cvclient::loop_client() {

    int bytes;
    int imgSize = no_image.total() * no_image.elemSize();



    while(ready) {
        cv::Mat *received = new cv::Mat();
        no_image.copyTo(*received);
        uchar *data = received -> data;
        while(framebuffer != NULL) {
            usleep(10000);
            if(!ready) break;
        }
        lock();
        if ((bytes = recv(sokt, data, imgSize , MSG_WAITALL)) <= 0) {
            //disconnect
            is_connected = false;
        }
        else {
            fails = 0;

            if(framebuffer != NULL) delete framebuffer;
            framebuffer = new cv::Mat();
            received -> copyTo(*framebuffer);
            delete received;
        }
        unlock();
        //usleep(10000);
    }

}

bool cvclient::is_ready() {
    return ready;
}




cv::Mat* cvclient::get_data() {

    if(framebuffer == NULL) return NULL;
    else {
        cv::Mat *res = new cv::Mat();
        lock();
        framebuffer -> copyTo(*res);
        delete framebuffer;
        framebuffer = NULL;
        unlock();
        return res;
    }
}

void cvclient::disconnect() {
    close(sokt);
    ready = false;
    if(client != NULL) {
        client -> join();
        delete client;
        client = NULL;
    }
}

bool cvclient::mat_is_valid(cv::Mat *m) {
	if(m == NULL) return false;
	else if(m -> size().width == 0 || m -> size().height == 0) return false;
	else return true;
}

bool cvclient::connected() {
    return is_connected;
}
