//
// Created by lorre851 on 10.03.17.
//

#include "cvclient.h"


cvclient::cvclient(int port, std::string ip) {
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
        if (connect(sokt, (sockaddr*)&serverAddr, addrLen) < 0) {
            std::cerr << "connect() failed! "<< serverIP << ":" << serverPort << std::endl;
        }
        else {
            client = new std::thread(&cvclient::loop_client, this);
            ready = true;
        }
    }
    //std::cout << "constructing done." << std::endl;
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

        while(framebuffer != NULL) usleep(10000);
        lock();
        if ((bytes = recv(sokt, data, imgSize , MSG_WAITALL)) <= 0) {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
            ready = false;
        }
        else {
            /*
            if(received -> isContinuous()) {
                buffer.push(received);
                if (buffer.size() > 30) {
                    //std::cout << "flushing buffer, size = " << buffer.size() << std::endl;
                    delete buffer.front();
                    buffer.pop();
                }
            }
            else {
                std::cout << "malicious frame dropped" << std::endl;
                delete received;
            }
            */

            if(framebuffer != NULL) delete framebuffer;
            framebuffer = new cv::Mat();
            received -> copyTo(*framebuffer);
            delete received;
        }
        unlock();
        usleep(10000);
    }

}

bool cvclient::is_ready() {
    return ready;
}


cv::Mat* cvclient::get_data() {
    /*
    //std::cout << "getting data" << std::endl;
    if(buffer.size() == 0) {
        //std::cout << "THERE WAS NO DATA" << std::endl;
	    return NULL;
    }
    cv::Mat *res = new cv::Mat();

    std::cout << "copying data: " << buffer.front() -> datastart << std::endl;
    //std::cout << "copying data from addr " << buffer.front() << std::endl;

    //TODO: decomission the buffer system. It is broken AF.
    buffer.front() -> copyTo(*res);
    lock();
    //std::cout << "copy done, removing buffer, size: " << buffer.size() << std::endl;
    delete buffer.front();
    buffer.pop();

    unlock();
    return res;
    */

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
}

bool cvclient::mat_is_valid(cv::Mat *m) {
	if(m == NULL) return false;
	else if(m -> size().width == 0 || m -> size().height == 0) return false;
	else return true;
}

