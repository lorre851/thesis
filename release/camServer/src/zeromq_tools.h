//
// Created by lorre851 on 16.03.17.
//

#ifndef ZEROMQ_ZEROMQ_TOOLS_H
#define ZEROMQ_ZEROMQ_TOOLS_H

#include <iostream>
#include <zmq.hpp>
#include <zconf.h>

class zeromq_tools {
    public:
        static std::string s_recv(zmq::socket_t &);
        static void s_send(const std::string &, zmq::socket_t &);
        static std::string hostname_to_ip(const char*);
        static long get_first_mac();
};

#endif //ZEROMQ_ZEROMQ_TOOLS_H
