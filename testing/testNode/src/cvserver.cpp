//
// Created by lorre851 on 01.03.17.
//


#include "cvserver.h"



cvserver::cvserver(int port) {
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;


    this -> port = port;
    putText(no_image, "cvserver on port " + to_string(this -> port) + " - no stream", Point(50, 50), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);


    init();
    if(ready) {
        listen(localSocket, 3);
        cout << "[cvserver] A new server has started on port " << this -> port << ", awaiting connections." << endl;
        server = new thread(&cvserver::loop_server, this);
    }
    else cout << "[cvserver] FATAL - could not start a server on port " << this -> port << "! Init aborted." << endl;

}

cvserver::~cvserver() {
    close_conn();
    if(server != NULL) server -> join();
    cout << "[cvserver] Terminated." << endl;
}

void cvserver::close_conn() {
    if(ready) {
        ready = false;
        shutdown(remoteSocket, 2);
        shutdown(localSocket, 2);
        close(remoteSocket);
        close(localSocket);
        cout << "[cvserver] Server closed." << endl;
        if(server != NULL) server -> join();
        server = NULL;
    }
}

void cvserver::init() {
    bool success = true;
    int sock_opt = 1;
    if(!ready) {
        //AF_INET: allow ipv4 connections (change to AF_INET6 for ipv6)
        //SOCK_STREAM: connection-based two-way byte stream
        //SOCK_NONBLOCK: make the accept() nonblocking so that we can exit the program at any point
        //MSG_NOSIGNAL: ubuntu crashes on client disconnect if this flag is not present.
        localSocket = socket(AF_INET , SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (localSocket == -1){
            cout << "[cvserver] <init> Failed to open socket!" << endl;
            success = false;
        }

        //Make the socket reusable, i.e. allow reconnect after disconnect
        if(setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int)) < 0) {
            cout << "[cvserver] <init> WARN: failed to set socket options! [0x01]" << endl;
        }

        //set timeout to 10 seconds for both send and receive
        if (setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            cout << "[cvserver] <init> WARN: failed to set socket options! [0x02]" << endl;
        }
        if (setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            cout << "[cvserver] <init> WARN: failed to set socket options! [0x03]" << endl;
        }

        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = INADDR_ANY;     //bind socket to all available interfaces
        localAddr.sin_port = htons(this -> port);

        if(bind(localSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
            cout << "[cvserver] <init> Failed to bind socket!" << endl;
            success = false;
        }


        //did everything go to plan?
        if(success) {
            ready = true;
            cout << "[cvserver] <init> Initialization succesful." << endl;
        }
        else cout << "[cvserver] <init> INIT FAILED!" << endl;
    }
    else cout << "[cvserver] <init> Already initialized." << endl;
}


void cvserver::loop_server() {
    int imgSize = 0;
    int bytes = 0;

    while(ready) {
        /*      Wait for connections        */
        while(ready) {
            //make a set of file descriptors and add the local socket to it
            fd_set set;
            int rv;
            FD_ZERO(&set);
            FD_SET(localSocket, &set);

            //reset the timeout - select(..) decrements the values to zero.. :(
            struct timeval wait;
            wait.tv_sec = 1;
            wait.tv_usec = 0;

            //wait for an incoming connection (rv == -1 for error or 0 for 'no request')
            rv = select(localSocket + 1, &set, NULL, NULL, &wait);
            if (rv > 0) {
                remoteSocket = accept(localSocket, (struct sockaddr *) &remoteAddr, (socklen_t *) &addrLen);
                if(remoteSocket >= 0) {
                    cout << "[cvserver] Client connected." << endl;
                    break;
                }
                else cout << "[cvserver] WARNING - failed to accept incoming connection!" << endl;
            }
        }



        /*      Connected - start sending stuff!        */
        while(ready) {
            //We are connected, let's push some stuff out

            while(buffer == NULL) {
                usleep(10000);
                if(!ready) break;
            }
            lock();



	        imgSize = buffer -> total() * buffer -> elemSize();

            if ((bytes = send(remoteSocket, buffer -> data, imgSize, MSG_NOSIGNAL)) < 0) {          //nosignal or crash :(
                /*          DISCONNECT           */
                cout << "[cvserver] Client disconnected." << endl;
                unlock();
                break;
            }
            delete buffer;
            buffer = NULL;
            unlock();
        }
    }


}

bool cvserver::is_ready() {
    return ready;
}

bool cvserver::push_data(const Mat *data) {

    /*
    delete image;
    */

    if(data != NULL) {
        
        Mat res, temp;
        resize(*data, res, Size(640, 360));
        cvtColor(res, temp, CV_BGR2GRAY);

        int watchdog = 0;
        while(buffer != NULL && ready) {
            usleep(100000);
            watchdog++;
            if(watchdog > 20) return false;
        }
        lock();
        buffer = new Mat();
        temp.copyTo(*buffer);
        unlock();
        return true;
    }
    else {
        return false;
    }

}

void cvserver::lock() {
    while(lock_data);
    lock_data = true;

}

void cvserver::unlock() {
    lock_data = false;
}
bool cvserver::toggle_debug() {
    debug = !debug;
    cout << "[cvserver] debug set to " << debug << endl;
    return debug;
}

void cvserver::reset() {
    cout << "[cvserver] Restarting..." << endl;
    close_conn();
    if(server != NULL) server -> join();
    init();
    if(!ready) cout << "[cvserver] FATAL: server failed to restore after client disconnect!" << endl;
    else {
        listen(localSocket, 3);
        server = new thread(&cvserver::loop_server, this);
        cout << "[cvserver] A new server has started on port " << this -> port << ", awaiting connections." << endl;
    }
}
