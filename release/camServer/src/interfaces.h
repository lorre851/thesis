//
// Created by lorre851 on 15.05.17.
//

#ifndef CAMSERVER_INTERFACES_H
#define CAMSERVER_INTERFACES_H

#include <iostream>
#include <vector>
#include <zmq.hpp>
#include "zeromq_tools.h"
#include "cvclient.h"
#include "depthpeopledetector.h"

/*
 *      INTERFACE STATUSCODES
 */

#define CAMSERVER_UNINITIALIZED -1
#define CAMSERVER_CONNECTED 0
#define CAMSERVER_UNRESOLVED 1
#define CAMSERVER_TCP_ERROR 2

#define CAMNODE_NOT_INITIALIZED -1
#define CAMNODE_CONNECTED 0
#define CAMNODE_INVALID_UUID 1
#define CAMNODE_INCOMPATIBLE 2
#define CAMNODE_UNRESOLVED 3
#define CAMNODE_TCP_ERROR 4


/*
 *      camServer interface
 *      used to send commands to other camServers
 */

class iface_server {
    private:
        std::string ip;                         //ip of the camServer
        int port;                               //TCP control port of camServer
        zmq::context_t *context;                //ZeroMQ context object (defines number of IO threads)
        zmq::socket_t *socket;                  //ZeroMQ socket object
        char tcpaddr[100];                      //c-string version of the full TCP address
        int status = CAMSERVER_UNINITIALIZED;   //current connection status
        std::string ID;                         //ZeroConf ID of the server

        volatile bool lock_channel = false;     //TCP control channel lock variable
        void lock();                            //lock socket
        void unlock();                          //unlock socket
        void reconnect();                       //reconnect to TCP control channel
        void init_socket();                     //initalize socket
        bool enabled = true;
    public:
        std::vector<std::string> camnodes;      //list of connected camNode's - needed when running master mode
        std::string get_uuid();                 //get the ZeroConf ID
        std::string command(std::string);       //send a command and receive the response

        int get_status();                       //returns connection status
        void disable();
        void enable();
        bool is_enabled();

        //constructor. IP, port, ZeroConf ID
        //TODO: change this to host, port, ZeroConf ID.
        iface_server(std::string, int, std::string);
};



/*
 *      camNode interface
 *      used to send commands to other camNodes
 */

class iface_node {
private:
    std::string ip;                             //ip of the camNode
    int tcp_port;                               //TCP control port of camServer
    zmq::context_t *context = NULL;             //ZeroMQ context object (defines number of IO threads)
    zmq::socket_t *socket = NULL;               //ZeroMQ socket object
    char tcpaddr[100];                          //c-string version of the full TCP address
    int status = CAMNODE_NOT_INITIALIZED;       //current connection status
    std::string uuid;                           //ZeroConf ID of the node
    int cv_port;                                //port for OpenCV video stream


    volatile bool lock_channel = false;         //TCP control channel lock variable
    void lock();                                //lock socket
    void unlock();                              //unlock socket
    void init_socket();                         //initialize socket
    void reconnect();                           //reconnect to socket

    //detection functions
    std::thread *detector = NULL;               //People detector thread
    depthpeopledetector *dpd = NULL;            //People detector object
    bool done = false;                          //Flag to check if results are available
    bool run_detection = false;
    void detect();                              //start people detector

    bool enabled = true;

public:
    std::string get_uuid();
    std::string command(std::string);
    int get_status();

    void disable();
    void enable();
    bool is_enabled();



    //TODO: this might be decommisioned soon
    static std::vector<std::string> explode(std::string const &, char);


    bool start_cvstream();
    bool stop_cvstream();
    cv::Mat* get_cvstream();



    //cvserver object
    cvclient *stream = NULL;


    //detection functions

    bool visualize = false;
    bool start_detection();
    bool stop_detection();
    bool is_detecting();
    bool results_available();       //returns done
    int get_in();
    int get_out();
    bool clear_results();

    //uid, hostname.

    iface_node(std::string, int, std::string);
    ~iface_node();
};

#endif //CAMSERVER_INTERFACES_H
