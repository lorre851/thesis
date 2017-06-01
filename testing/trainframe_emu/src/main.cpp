#include <iostream>
#include <thread>
#include "zeromq_tools.h"
#include "servus/servus.h"
#include "interfaces.h"

#define NONE -1
#define BROADCAST -2
#define CAMSERVER 1
#define CAMNODE 2


using namespace std;

int current_interface = CAMSERVER;
int current_mode = BROADCAST;


bool searchservers = true;

/*
 *      camServer container for master
 *      used to send commands to camServers
 */



vector<iface_server> camserver_interfaces;
vector<iface_node> camnode_interfaces;

void find_camservers(servus::Servus &s) {
    while(searchservers) {
        servus::Strings list = s.discover(s.IF_ALL, 1000);
        for (int i = 0; i < list.size(); i++) {
            bool found = false;
            for (int j = 0; j < camserver_interfaces.size(); j++) {

                if (camserver_interfaces[j].get_uuid() == list[i]) {
                    found = true;
                    j = camserver_interfaces.size();
                }
            }
            if (!found) {
                camserver_interfaces.push_back(iface_server(s.getHost(list[i]), stoi(s.get(list[i], "tcp_port")), list[i]));
                cout << "[found server] " << list[i] << endl;
            }

        }

        //kill dead servers
        std::vector<iface_server>::iterator begin = camserver_interfaces.begin(), end = camserver_interfaces.end();

        while(begin != end) {
            if(begin -> command("ping") != "pong") {
                std::cout << "[lost server] " << begin -> get_uuid() << std::endl;
                camserver_interfaces.erase(begin);
            }
            begin++;
        }



    }
}

void find_camnodes(servus::Servus &s) {
    while(searchservers) {
        servus::Strings list = s.discover(s.IF_ALL, 1000);
        for (int i = 0; i < list.size(); i++) {
            bool found = false;
            for (int j = 0; j < camnode_interfaces.size(); j++) {

                if (camnode_interfaces[j].get_uuid() == list[i]) {
                    found = true;
                    j = camnode_interfaces.size();
                }
            }
            if (!found) {
                camnode_interfaces.push_back(iface_node(s.getHost(list[i]), stoi(s.get(list[i], "tcp_port")), (std::string)list[i]));
                cout << "[found node] " << list[i] << endl;
            }

        }

        //kill dead nodes
        std::vector<iface_node>::iterator begin = camnode_interfaces.begin(), end = camnode_interfaces.end();

        while(begin != end) {
            if(begin -> command("ping") != "pong") {
                std::cout << "[lost node] " << begin -> get_uuid() << std::endl;
                camnode_interfaces.erase(begin);
            }
            begin++;
        }



    }

}

int main() {
    cout << "trainframe // train mainframe emulator" << endl;
    cout << "type '?' for a command list" << endl;
    cout << "----------------------------------------" << endl;
    servus::Servus s("_camserver._tcp");
    servus::Servus t("_camnode._tcp");

    cout << "looking for components..." << endl;
    thread scanner(find_camservers, ref(s));
    thread scanner2(find_camnodes, ref(t));

    sleep(1);

    cout << "ready! type '?' for a command list or 'switch' to switch interfaces." << endl;
    cout << "Selected camServer interface in broadcast mode." << endl;

    string cmd = "";
    while(cmd != "quit") {
        getline(cin, cmd);
        vector<string> parser = iface_node::explode(cmd, ' ');

        if(cmd == "quit") break;

        else if(parser[0] == "list") {
            if(current_interface == CAMSERVER) {
                cout << "-- camServer list --" << endl;
                for (int i = 0; i < camserver_interfaces.size(); i++)
                    cout << "[" << i << "] " << camserver_interfaces[i].get_uuid() << " @ " << camserver_interfaces[i].get_hostname() << " on " << camserver_interfaces[i].get_ip() << endl;
                cout << "-- end of list" << endl;
            }
            else {
                cout << "-- camNode list --" << endl;
                for (int i = 0; i < camnode_interfaces.size(); i++)
                    cout << "[" << i << "] " << camnode_interfaces[i].get_uuid() << " @ " << camnode_interfaces[i].get_hostname() << " on " << camnode_interfaces[i].get_ip() << endl;
                cout << "-- end of list" << endl;
            }
        }
        else if(parser[0] == "broadcast") {
            current_mode = BROADCAST;
            cout << "switched to broadcast mode" << endl;
        }
        else if(parser[0] == "interface") {
            if(parser.size() != 2) cerr << "Expected argument: ID" << endl;
            else {
                int iface;
                try {
                    iface = stoi(parser[1]);
                    if(current_interface == CAMSERVER) {
                        if (iface < camserver_interfaces.size() && iface >= 0) {
                            current_mode = iface;
                            cout << "switched to interface " << iface << endl;
                        } else cerr << "Invalid interface selected. Use list." << endl;
                    }
                    else {
                        if (iface < camnode_interfaces.size() && iface >= 0) {
                            current_mode = iface;
                            cout << "switched to interface " << iface << endl;
                        } else cerr << "Invalid interface selected. Use list." << endl;
                    }
                }
                catch(invalid_argument e) {
                    cerr << "Invalid argument: ID. Expected a number." << endl;
                }
            }
        }
        else if(parser[0] == "switch") {
            if (current_interface == CAMSERVER) {
                current_interface = CAMNODE;
                current_mode = BROADCAST;
                cout << "switched to camnode broadcast" << endl;
            }
            else {
                current_interface = CAMSERVER;
                current_mode = BROADCAST;
                cout << "switched to camserver broadcast" << endl;
            }
        }
        else if(parser[0] == "?") {
            cout << endl;
            cout << "command list" << endl;
            cout << "--------------------" << endl;
            cout << "quit - exit trainframe emulator" << endl;
            cout << "list - get a list of interfaces" << endl;
            cout << "interface [number] - select an interface" << endl;
            cout << "broadcast - start broadcast mode" << endl;
            cout << "switch - switch from camServer / camNode interface" << endl;
            cout << "*litteraly anything else* - will be sent to the selected interface" << endl;
            cout << endl;
        }
        else if(current_mode != NONE) {
            if(current_interface == CAMSERVER) {
                if (current_mode == BROADCAST) {
                    for (int i = 0; i < camserver_interfaces.size(); i++) {
                        string recv = camserver_interfaces[i].command(cmd);
                        if (recv == "") cerr << "server down: " << i << endl;
                        else cout << "[" << i << "] recv: " << recv << endl;
                    }
                } else {
                    string recv = camserver_interfaces[current_mode].command(cmd);
                    if (recv == "") cerr << "server down: " << current_mode << endl;
                    else cout << "recv: " << recv << endl;
                }
            }
            else {
                if (current_mode == BROADCAST) {
                    for (int i = 0; i < camnode_interfaces.size(); i++) {
                        string recv = camnode_interfaces[i].command(cmd);
                        if (recv == "") cerr << "node down: " << i << endl;
                        else cout << "[" << i << "] recv: " << recv << endl;
                    }
                }
                else {
                    string recv = camnode_interfaces[current_mode].command(cmd);
                    if (recv == "") cerr << "server down: " << current_mode << endl;
                    else cout << "recv: " << recv << endl;
                }
            }
        }
        else cerr << "no interface selected! command aborted." << endl;




    }
    searchservers = false;
    cout << "quitting..." << endl;
    scanner.join();
    scanner2.join();

    return 0;
}
