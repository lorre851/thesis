//
// Created by lorre851 on 16.03.17.
//


#include <netdb.h>
#include <arpa/inet.h>
#include "zeromq_tools.h"


std::string zeromq_tools::s_recv(zmq::socket_t & socket) {
    int retry = 0;
    zmq::message_t message;
    int status = socket.recv(&message, ZMQ_NOBLOCK);
    while(status == 0 && retry++ < 5) {
        usleep(10000);
        status = socket.recv(&message, ZMQ_NOBLOCK);
    }
    return std::string(static_cast<char*>(message.data()), message.size());
}

void zeromq_tools::s_send(const std::string &str, zmq::socket_t &socket) {
    int retry = 0;
    zmq::message_t reply(str.length());
    memcpy(reply.data(), str.c_str(), str.length());
    int status = socket.send(reply, ZMQ_NOBLOCK);
    while(status == 0 && retry++ < 5) {
        usleep(10000);
        status = socket.send(reply, ZMQ_NOBLOCK);
    }
}

std::string zeromq_tools::hostname_to_ip(const char* hostname) {
    char ch_ip[100];            //strcpy doesn't work in char *ch_ip, it just copies onto empty memory then...
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL) {
        return "ERR_1";
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        strcpy(ch_ip , inet_ntoa(*addr_list[i]) );

        return std::string(ch_ip);
    }

    return "ERR_2";
}