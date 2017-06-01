//
// Created by lorre851 on 16.03.17.
//

// Revision 1 - s_recv and s_send implementation
// Revision 2 - Added IP address resolver
// Revision 3 - Increased attempt count to 50 and decreased timeout to 10ms
// Revision 4 - Added MAC addr interface detector
// Revision 5 - added try-catch around zmq functions - we don't want them taking down the program


#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fstream>
#include "zeromq_tools.h"


std::string zeromq_tools::s_recv(zmq::socket_t & socket) {

    int retry = 0, status = 0;
    zmq::message_t message;
    try {
        status = socket.recv(&message, ZMQ_NOBLOCK);
    }
    catch(zmq::error_t e) {
        status = 0;
    }
    while (status == 0 && retry++ < 50) {
        usleep(10000);
        try {
            status = socket.recv(&message, ZMQ_NOBLOCK);
        }
        catch(zmq::error_t e) {
            status = 0;
        }
    }
    return std::string(static_cast<char *>(message.data()), message.size());

}

void zeromq_tools::s_send(const std::string &str, zmq::socket_t &socket) {
    int retry = 0, status = 0;
    zmq::message_t reply(str.length());
    memcpy(reply.data(), str.c_str(), str.length());
    try {
        status = socket.send(reply, ZMQ_NOBLOCK);
    }
    catch(zmq::error_t e) {
        status = 0;
    }
    while(status == 0 && retry++ < 50) {
        usleep(10000);
        try {
            status = socket.send(reply, ZMQ_NOBLOCK);
        }
        catch(zmq::error_t e) {
            status = 0;
        }
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

            int integers = 0;
            for(int i = 0; i < name.size(); i++) {
                if(integers == 8) break;
                if(name[i] == ':') {
                    //do nothing
                }
                else if(isdigit(name[i])) {
                    name_numeric += name[i];
                    integers++;
                }
                else {
                    name_numeric += std::to_string((int)name[i] % 10);
                    integers++;
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