//
// Created by lorre851 on 15.05.17.
//

#include "interfaces.h"

/*
 *      camServer interface
 *
 */

/*
 *      PRIVATE FUNCTIONS
 */

void iface_server::reconnect() {
    try {
        socket -> disconnect(tcpaddr);
        socket -> close();
        context -> close();
    }
    catch(zmq::error_t) {

    }

    delete context;
    delete socket;
    init_socket();
}

void iface_server::init_socket() {
    context = new zmq::context_t(ZMQ_SUB);
    socket = new zmq::socket_t(*context, ZMQ_REQ);
    socket -> setsockopt(ZMQ_LINGER, 0);               //FINALLY... http://www.loadingartist.com/comic/rest-assured/
    socket -> connect(tcpaddr);
}



void iface_server::lock() {
    while(lock_channel);
    lock_channel = true;
}

void iface_server::unlock() {
    lock_channel = false;
}

/*
 *      PUBLIC FUNCTIONS
 */
//constructor
iface_server::iface_server(std::string host, int port, std::string ID) {
    this -> ip = zeromq_tools::hostname_to_ip(host.c_str());
    this -> port = port;
    this -> ID = ID;
    this -> hostname = host;

    if(ip.compare("ERR_1") == 0 || ip.compare("ERR_2") == 0) {
        status = CAMSERVER_UNRESOLVED;
    }
    else {
        //parse TCP address and put it in a C - string
        std::string addr = "tcp://" + ip + ":" + std::to_string(this->port);
        strcpy(tcpaddr, addr.c_str());
        tcpaddr[99] = '\0';         //terminate string in case something went wrong here

        //start context and socket
        init_socket();

        //check if connected
        if (socket->connected()) {
            if (this->command("ping") != "pong") {
                std::cerr << "[cserver_construct] warning, did not receive anything! " << addr << std::endl;
                status = CAMSERVER_TCP_ERROR;
            }

        } else {
            std::cerr << "[cserver_construct] warning, no connection!" << addr << std::endl;
            status = CAMSERVER_TCP_ERROR;
        }
    }
}


std::string iface_server::get_uuid() {
    return ID;
}

std::string iface_server::get_hostname() {
    return hostname;
}

std::string iface_server::get_ip() {
    return std::string(tcpaddr);
}

std::string iface_server::command(std::string cmd) {
    try {
        lock();
        zeromq_tools::s_send(cmd, *socket);
        std::string recv = zeromq_tools::s_recv(*socket);


        //if receiving length is zero, retry 3 times.
        if (recv.length() == 0) {
            int i;
            for (i = 0; i < 3; i++) {
                usleep(250000);
                recv = zeromq_tools::s_recv(*socket);
                if (recv.length() > 0) i = 4;

            }
            if (i == 3) {
                status = CAMSERVER_TCP_ERROR;
                unlock();
                return "err_01";
            };
        }
        status = CAMSERVER_CONNECTED;
        unlock();
        return recv;
    }
    catch(zmq::error_t e) {
        //sending a command without receiving a response results in an error
        status = CAMSERVER_TCP_ERROR;
        unlock();
        return "dead";
    }
}





int iface_server::get_status() {
    if(status == CAMSERVER_TCP_ERROR) {
        reconnect();
        zeromq_tools::s_send("ping", *socket);
        std::string recv = zeromq_tools::s_recv(*socket);
        if(recv.compare("pong") == 0) status = CAMSERVER_CONNECTED;
    }


    return status;
}


void iface_server::disable() {
    enabled = false;
}

void iface_server::enable() {
    enabled = true;
}

bool iface_server::is_enabled() {
    return enabled;
}

/*
 *      camNode interface
 *
 */

void iface_node::lock() {
    while(lock_channel);
    lock_channel = true;
}

void iface_node::unlock() {
    lock_channel = false;
}

iface_node::iface_node(std::string host, int port, std::string uid) {
    uuid = uid;
    tcp_port = port;
    ip = zeromq_tools::hostname_to_ip(host.c_str());      //THIS BOOGER IS MAKING MY LIFE A LIVING HELL
    hostname = host;

    if(ip.compare("ERR_1") == 0 || ip.compare("ERR_2") == 0) {
        status = CAMNODE_UNRESOLVED;
    }
    else {
        //start tcp connection
        std::string addr = "tcp://" + ip + ":" + std::to_string(tcp_port);
        strcpy(tcpaddr, addr.c_str());
        tcpaddr[99] = '\0';         //terminate string in case something went wrong here

        init_socket();


        if(socket -> connected()) {
            zeromq_tools::s_send("ping", *socket);
            std::string recv = zeromq_tools::s_recv(*socket);
            if(recv.compare("pong") == 0) {
                status = CAMNODE_CONNECTED;
                //connected, ask for the cvserver port
                zeromq_tools::s_send("port", *socket);
                recv = zeromq_tools::s_recv(*socket);
                if(status == CAMNODE_CONNECTED) {
                    cv_port = atoi(recv.c_str());
                }
            }
            else status = CAMNODE_TCP_ERROR;
        }
        else status = CAMNODE_TCP_ERROR;

    }






}

iface_node::~iface_node() {

}

void iface_node::init_socket() {
    context = new zmq::context_t(ZMQ_SUB);
    socket = new zmq::socket_t(*context, ZMQ_REQ);
    socket -> setsockopt(ZMQ_LINGER, 0);               //FINALLY... http://www.loadingartist.com/comic/rest-assured/
    socket -> connect(tcpaddr);
}

void iface_node::reconnect() {
    try {
        socket -> disconnect(tcpaddr);
        socket -> close();
        context -> close();
    }
    catch(zmq::error_t) {

    }

    delete context;
    delete socket;
    init_socket();
}

int iface_node::get_status() {
    //this status may be recoverable
    if(status == CAMNODE_TCP_ERROR) {
        reconnect();
        zeromq_tools::s_send("ping", *socket);
        std::string recv = zeromq_tools::s_recv(*socket);
        if(recv.compare("pong") == 0) {
            status = CAMNODE_CONNECTED;
        }
    }

    return status;
}


//thank you stackoverflow
std::vector<std::string> iface_node::explode(std::string const &s, char delim) {
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); ) {
        result.push_back(std::move(token));
    }

    return result;
}





std::string iface_node::command(std::string cmd) {
    try {
        lock();
        if(status != CAMNODE_CONNECTED) {
            unlock();
            return "err_03";
        }

        zeromq_tools::s_send(cmd, *socket);
        std::string recv = zeromq_tools::s_recv(*socket);


        //if receiving length is zero, retry 3 times.
        if (recv.length() == 0) {
            int i;
            for (i = 0; i < 3; i++) {
                usleep(250000);
                recv = zeromq_tools::s_recv(*socket);
                if (recv.length() > 0) i = 4;

            }
            if (i == 3) {
                status = CAMNODE_TCP_ERROR;
                unlock();
                return "err_01";
            };
        }
        unlock();
        return recv;
    }
    catch(zmq::error_t ex) {
        std::cerr << "[camnode] Exception while sending command, node kicked." << std::endl;
        std::cerr << ex.what() << std::endl;
        status = CAMNODE_TCP_ERROR;

        unlock();
        return "err_02";
    }

}


bool iface_node::start_cvstream() {
    if(stream == NULL) {
        stream = new cvclient(cv_port, ip);
        if(stream -> is_ready()) {
            return true;
        }
        else {
            stop_cvstream();
            return false;
        }
    }
    else return false;
}

bool iface_node::stop_cvstream() {
    if(stream != NULL) {
        stream -> disconnect();
        delete stream;
        stream = NULL;
        return true;
    }
    else return false;
}

cv::Mat* iface_node::get_cvstream() {
    if(stream != NULL) return stream -> get_data();
}

std::string iface_node::get_uuid() {
    return uuid;
}

std::string iface_node::get_hostname() {
    return hostname;
}

std::string iface_node::get_ip() {
    return std::string(tcpaddr);
}



void iface_node::detect() {
    while(run_detection && get_status() == CAMNODE_CONNECTED) {
        if(command("ping") == "pong") {
            cv::Mat *get_frame = stream -> get_data();
            if (cvclient::mat_is_valid(get_frame)) {
                dpd -> visualize(visualize);
                dpd -> detect(*get_frame);
                delete get_frame;
            }
        }
        else {

            std::cerr << "[camnode] <detect> lost " << get_uuid() << std::endl;
            //cam -> stream -> disconnect();

            //You cannot call stop_detection here, it waits for detect() to end
            //system will notice this deadlock and kill the program.
            //..or call it asynchronous

            new std::thread(&iface_node::stop_detection, this);


        }
    }
}




bool iface_node::start_detection() {
    if(get_status() != CAMNODE_CONNECTED) {
        std::cerr << "[camserver] Detection not started on dead " << get_uuid() << std::endl;
        return false;
    }
    else if(!done && !run_detection && dpd == NULL) {
        /*
         *      remember how camNode handles the mode command?
         *
         *      1 * MAIN_FLAG_stereo + 2 * MAIN_FLAG_doflip + 4 * MAIN_FLAG_STREAMGRID + 8 * MAIN_FLAG_getcam
         *
         *      Stereo is [& 1]
         *
         */


        std::string recv = command("mode");

        try {
            int mode = stoi(recv);                //if mode & 8 -> stereostream enabled
            if (!(mode & 1)) {
                command("stereo");
                recv = command("mode");
                mode = stoi(recv);
                if (!(mode & 1)) {
                    std::cerr << "[camnode] mode could not be set on camnode " << get_uuid() << ", got " << mode
                              << std::endl;
                    return false;
                }
            }
        }
        catch(std::invalid_argument i) {
            std::cerr << "[camnode] Illegal response from camnode while setting mode: " << recv << std::endl;
            return false;
        }

        dpd = new depthpeopledetector();
        start_cvstream();
        run_detection = true;
        detector = new std::thread(&iface_node::detect, this);
        //std::cout << "[camserver] detection started: " << cam -> get_uuid() << std::endl;
        return true;
    }
    else {
        std::cerr << "[camnode] detection already started on " << get_uuid() << std::endl;
        return false;
    }

}

bool iface_node::stop_detection() {
    if(run_detection) {
        run_detection = false;
        if(detector != NULL) {
            detector -> join();
            delete detector;
            detector = NULL;
        }
        stop_cvstream();
        dpd -> cleanup();
        done = true;
    }
    else return false;
}

bool iface_node::is_detecting() {
    return run_detection;
}

int iface_node::get_in() {
    if(done) {
        return dpd -> results_in();
    }
    else return -1;
}

int iface_node::get_out() {
    if(done) {
        return dpd -> results_out();
    }
    else return -1;
}

bool iface_node::clear_results() {
    delete dpd;
    dpd = NULL;
    done = false;
}

bool iface_node::results_available() {
    return done;
}

void iface_node::disable() {
    enabled = false;
}

void iface_node::enable() {
    enabled = true;
}

bool iface_node::is_enabled() {
    return enabled;
}