#include "linux/kd.h"
#include "camerascanner.h"
#include "streamgrid.h"
#include "stereotools.h"
#include "camera_calibration.cpp"
#include "cvserver.h"
#include "zeromq_tools.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <servus/servus.h>
#include <random>





/*
 *
 *      INTERNAL FLAGS
 *
 */

bool MAIN_FLAG_capture = false;                             //stereostream calibration capture flag (one frame)
bool MAIN_FLAG_srun = true;                                 //main thread run flag
bool MAIN_FLAG_SWAP = false;                                //Swap mode (swap first two camera's)


/*
 *
 *      OPERATION MODES
 *
 */

bool MAIN_FLAG_stereo = true;                               //generate depth map
bool MAIN_FLAG_doflip = false;                              //flip mode (implies calibration wizard mode)
bool MAIN_FLAG_STREAMGRID = false;                          //enables streamgrid (up to 4 streams)
bool MAIN_FLAG_getcam = false;                              //stream webcam instead of depthmap (disables stereo)


/*
 *
 *      INTERNAL PARAMETERS
 *
 */

const string CAMNODE_VERSION = "0.1";
string calibration_write = "";                              //buffer for calibration xml
int MAIN_FLAG_capcount = 0;                                 //counter for number of captured calibration frames
Mat *temp1, *temp2;                                         //temp mats
int JANKY_WEBCAM_CONSTANT = 1;                              //for nowebcam mode
string dump_path = "/";           //main path for storing everything
string tcperr_last = "ok";
const string STREAMGRID_TITLE = "camServer Tester " + CAMNODE_VERSION;           //application splash title
int fd = open("/dev/console", O_RDONLY);



/*
 *
 *      SETTINGS (cannot be modified at runtime)
 *
 */

Size MAIN_CALSIZE(6, 9);                                    //Size of calibration checkerboard
float MAIN_BLOCKSIZE = 50.0;                                //Size (mm) of one checkerboard block
int port = 9420;                                            //Stereostream / webcamstream port
int tcp_port = 5555;                                        //TCP control port
bool MAIN_FLAG_tcpctrl = true;                              //start with TCP control channel enabled (disables std::cin)
bool MAIN_FLAG_nowebcams = false;                           //start with cam0 disabled (if internal webcam is present)
bool MAIN_FLAG_cvserver = true;                             //start with webcam server enabled




int spoof_mode = 8;

string stream_name = "";

//A random generator
std::random_device rd;
std::mt19937 getrandom(rd());
int sleep_period = 75000;



/*********************************************************************
 *
 *                      MAIN WORKER THREAD
 *
 *********************************************************************/



void push_frames(cvserver &server, bool memload) {


    if(memload) {
	cout << "starting stream: " << stream_name << endl;
	VideoCapture cap(stream_name);
	vector<Mat*> frames;
	Mat *buffer = new Mat();
	cout << "##### STARTED IN MEMORY MODE - loading frames into memory #####" << endl;
	while(cap.read(*buffer)) {
		Mat *frame = new Mat();
		buffer -> copyTo(*frame);
		delete buffer;
		buffer = new Mat();
		frames.push_back(frame);
	}
	cout << "done. starting stream..." << endl;


	int count = 0;
	int framecount = 0;
	time_t tstart;
	time_t tend;
	while(MAIN_FLAG_srun) {
		if(framecount == 0) time(&tstart);
		framecount++;
		if(count >= frames.size()) count = 0;
		server.push_data(frames[count++]);
		if(framecount == 30) {
			time(&tend);
			double fps = 30.0 / (tend - tstart);
			framecount = 0;
			cout << "avg: " << fps << " fps" << endl;
		}

        usleep(sleep_period);
	}
    }
    else {
	cout << "starting stream: " << stream_name << endl;
	VideoCapture cap(stream_name);
	int framecount = 0;
	time_t tstart;
	time_t tend;
	while(MAIN_FLAG_srun) {
		if(framecount == 0) time(&tstart);
		framecount++;
		Mat *frame = new Mat();

		if(!cap.read(*frame)) {
		    cout << "resetting position" << endl;
		    cap.set(CAP_PROP_POS_FRAMES, 0);
		    cap.read(*frame);

		}

		server.push_data(frame);
		delete frame;
		if(framecount == 60) {
			time(&tend);
			double fps = 60.0 / (tend - tstart);
			framecount = 0;
			cout << "avg: " << fps << " fps" << endl;
		}
        usleep(sleep_period);
	}
    }
}


/*********************************************************************
 *
 *                      main()
 *
 *********************************************************************/


int main(int argc, char **argv) {
    cout << STREAMGRID_TITLE << endl;
    cout << "(c) 2016, 2017 - Laurent Loots - UGent - IBCN" << endl;
    cout << endl << "####################################################################" << endl << endl;



    bool memload = false;

    /*      Process command line parameters     */
    if(argc >= 3) {
        if(!config::file_exists(string(argv[2]))) {
            cout << "Error: file not found!" << endl;
        }
        tcp_port = atoi(argv[1]);
        port = tcp_port + 1;
        stream_name = string(argv[2]);
    if(argc == 4) sleep_period = atoi(argv[3]);
	if(argc == 5) memload = true;
    }
    else {
        cout << "Usage: testsuite port path_to_video [ms per frame] [load in memory]" << endl;
        return -420;
    }





    //start cvserver if asked
    cvserver *server = new cvserver(port);



    //start tcp control channel if asked
    zmq::context_t context(1);
    zmq::socket_t tcpsocket(context, ZMQ_REP);
    if(MAIN_FLAG_tcpctrl) {
        tcpsocket.bind("tcp://*:"+to_string(tcp_port));
        cout << "[main] TCP control channel started on port " << tcp_port << endl;
    }


    //announce yourself
    servus::Servus s("_camnode._tcp");
    s.set("tcp_port", to_string(tcp_port));
    string uid = to_string(getrandom());

    cout << "[main] Zeroconf announcement: " << s.announce(100, uid) << endl;
    cout << "[main] Announced as " + uid << endl;



    thread *thr = new thread(push_frames, ref(*server), ref(memload));
    //all done!
    cout << "[main] Ready!" << endl;



    //main loop: fetch commands
    bool run = true;
    string cmd;
    while(run) {

        /*
         *          FETCH COMMAND
         */
        if(MAIN_FLAG_tcpctrl) {
            cmd = zeromq_tools::s_recv(tcpsocket);
            while(cmd.length() == 0) {
                cmd = zeromq_tools::s_recv(tcpsocket);
            }

            if(cmd == "errno") {
                zeromq_tools::s_send(tcperr_last, tcpsocket);
                tcperr_last = "ok";
                cmd = "nop";
            }
            else if(cmd == "mode") {
                zeromq_tools::s_send(to_string(spoof_mode), tcpsocket);
                cmd = "nop";
            }
            else if(cmd == "ping") {
                zeromq_tools::s_send("pong", tcpsocket);
                cmd = "nop";
            }
            else if(cmd == "port") {
                zeromq_tools::s_send(to_string(port), tcpsocket);
                cmd = "nop";
            }

            else {
                zeromq_tools::s_send("ack", tcpsocket);
            }
        }

        else {
            cin >> cmd;
        }


        /*
         *          PARSE COMMAND
         */
        if(cmd == "exit") {
            run = false;
            cout << "[main] Terminating threads..." << endl;
            break;
        }

        else if(cmd == "cvsrestart") {
            if(MAIN_FLAG_cvserver) server -> reset();
            else cout << "[main] cvserver disabled, change program arguments and restart" << endl;
        }

        else if(cmd == "stereo") {
            //set adequate mode
            spoof_mode = 1;
        }

        else if(cmd == "cam") {
            spoof_mode = 8;
        }
        else if(cmd == "nop") {
            //nope
        }
        else {
            cout << "Unknown command: '" << cmd << "'." << endl;
            tcperr_last = "CMD_UNKNOWN";
        }

    }


    MAIN_FLAG_srun = false;
    if(MAIN_FLAG_cvserver) server -> close_conn();

    cout << "[main] *** goodbye ***" << endl;
    return 0;

}

