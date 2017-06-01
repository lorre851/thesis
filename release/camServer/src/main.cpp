#include <iostream>
#include "depthpeopledetector.h"
#include "camserver.h"
#include "config.h"


using namespace std;




int main(int argc, char** argv) {

    //get config path
    string path;
    if(argc == 1) path = string(argv[0]);
    else if(argc == 2) path = string(argv[1]);
    path = config::trailingslash(path);

    int port = 0;
    if(argc == 2) port = atoi(argv[1]);

    camserver c(port, path);

    string cmd = "";
    while(cmd != "exit") {
        getline(cin, cmd);
        vector<string> parser = iface_node::explode(cmd, ' ');

        if(parser[0] == "list") c.get_interfaces();
        else if(parser[0] == "interface") {
            if(parser.size() == 2) {
                try {
                    c.select_interface(stoi(parser[1]));
                }
                catch(invalid_argument i) {
                    cerr << "number expected" << endl;
                }
            }
            else cerr << "argument expected" << endl;
        }
        else if(parser[0] == "clear") {
            cv::destroyAllWindows();
            cv::waitKey(10);
        }
        else if(parser[0] == "help") {
            cout << endl;
            cout << "command list" << endl;
            cout << "--------------------" << endl;
            cout << "list - display a list of camNodes" << endl;
            cout << "interface [id] - open a camNode feed" << endl;
            cout << "clear - destroy all OpenCV windows" << endl;
            cout << "help - display command list" << endl;
            cout << "exit - terminate program" << endl;
            cout << endl;
        }
        else if(parser[0] == "exit") break;
        else cerr << "Invalid command. Type 'help' for command list." << endl;

    }

    return 0;



}
