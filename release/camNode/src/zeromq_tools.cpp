//
// Created by lorre851 on 16.03.17.
//


#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fstream>
#include "zeromq_tools.h"


std::string zeromq_tools::s_recv(zmq::socket_t & socket) {
    int retry = 0;
    zmq::message_t message;
    int status = socket.recv(&message, ZMQ_NOBLOCK);
    while(status == 0 && retry++ < 50) {
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
    while(status == 0 && retry++ < 50) {
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

long zeromq_tools::get_first_mac() {
    std::string net = "/sys/class/net/";
    std::string interface = "";
    std::string name = "";
    std::string name_numeric = "";
    DIR *dir;
    struct dirent *ent;
    //get a list of network interfaces
    if((dir = opendir (net.c_str())) != NULL) {
        /* print all the files and directories within directory */
        int test = 0;
        while ((ent = readdir (dir)) != NULL) {
            //discard directory up and the loopback interface
            if(strcmp(ent -> d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, "lo") != 0) {
                interface = std::string(ent -> d_name);
                break;
            }
        }
        closedir(dir);

        if(interface != "") {
            std::ifstream myfile;
            myfile.open (net+interface+"/address");
            myfile >> name;

            for(int i = 0; i < name.size(); i++) {
                if(name[i] == ':') {
                    //do nothing
                }
                else if(isdigit(name[i])) {
                    name_numeric += name[i];
                }
                else {
                    name_numeric += std::to_string((int)name[i] % 10);
                }
            }
            return stol(name_numeric);
        }
        else return -1;
    }
    else {
        return -2;
    }
}