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

bool MAIN_FLAG_stereo = false;                               //generate depth map
bool MAIN_FLAG_doflip = false;                              //flip mode (implies calibration wizard mode)
bool MAIN_FLAG_STREAMGRID = false;                          //enables streamgrid (up to 4 streams)
bool MAIN_FLAG_getcam = false;                              //stream webcam instead of depthmap (disables stereo)


/*
 *
 *      INTERNAL PARAMETERS
 *
 */

const string CAMNODE_VERSION = "0.5.1";
string calibration_write = "";                              //buffer for calibration xml
int MAIN_FLAG_capcount = 0;                                 //counter for number of captured calibration frames
Mat *temp1, *temp2;                                         //temp mats
int JANKY_WEBCAM_CONSTANT = 1;                              //for nowebcam mode
string dump_path = "/";           //main path for storing everything
string tcperr_last = "ok";
const string STREAMGRID_TITLE = "camNode Software " + CAMNODE_VERSION;           //application splash title



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






/*********************************************************************
 *
 *                      MAIN WORKER THREAD
 *
 *********************************************************************/


void stereostream(camerascanner &sc, streamgrid &g, stereotools &ster, cvserver &server) {
    Mat *m1;
    Mat *m2;
    Mat *stereo = new Mat(), *gray;

    int timeout;
    while(MAIN_FLAG_srun) {

        timeout = 50;       //50ms = 20fps


        /*
         *      MAIN_FLAG_stereo
         *      Enable stereo image processing and depth map generation
         */
        if(MAIN_FLAG_stereo) {
            if (!ster.validate_config()) {
                cout << "[main] <stereowrapper> No stereovision config was loaded! Please calibrate." << endl;
                cout << "[main] <stereowrapper> - STEREO OFF -" << endl;
                tcperr_last = "NO_CAL";
                MAIN_FLAG_stereo = false;
            }
            else {
                //Explicitly grab a stereo frame
                Mat **st = sc.get_stereo_frame(2 - JANKY_WEBCAM_CONSTANT, 1 - JANKY_WEBCAM_CONSTANT);
                if (st == NULL) {
                    cout << "[main] <stereowrapper> Stereo frame was not available! Are you sure you plugged in two cameras?" << endl;
                    cout << "[main] <stereowrapper> - STEREO OFF -" << endl;
                    MAIN_FLAG_stereo = false;
                }
                else {
                    temp1 = new Mat();
                    temp2 = new Mat();
                    //a little denoising
                    GaussianBlur(*st[0], *temp1, Size(9, 9), 1, 1);
                    GaussianBlur(*st[1], *temp2, Size(9, 9), 1, 1);
                    //for debugging purposes: if the depth map looks like all hell, swap the camera positions (left / right)
                    //STEREO uses Semi Global Block Matching, see reference: http://zone.ni.com/reference/en-XX/help/372916M-01/nivisionconceptsdita/guid-53310181-e4af-4093-bba1-f80b8c5da2f4/
                    if (!MAIN_FLAG_SWAP) gray = ster.generate_depth_map(*temp1, *temp2);
                    else gray = ster.generate_depth_map(*temp2, *temp1);
                    //conversion for streamgrid
                    cvtColor(*gray, *stereo, CV_GRAY2BGR);
                    if(MAIN_FLAG_cvserver) {
                        server.push_data(stereo);
                    }

                    //push to streamgrid if asked

                    try {
                        g.set_block(*st[0], 0);
                        g.set_block(*st[1], 1);
                        g.draw();
                    }
                    catch (Exception ex) {
                        cout << "[main] EXCEPTION in streamgrid" << endl;
                        tcperr_last = "STREAMGRID_EXCEPTION";
                        cout << ex.what() << endl;
                    }

                    //don't forget the great leakage of 27/2/2017
                    delete[] st;
                    delete gray;
                    delete temp1;
                    delete temp2;
                }
            }
        }

        if(MAIN_FLAG_getcam) {
            if(MAIN_FLAG_cvserver) {
                Mat *frame = sc.frame(1 - JANKY_WEBCAM_CONSTANT);
                server.push_data(frame);
                delete frame;
            }
        }

        if(MAIN_FLAG_doflip) {
            sc.set_grab_speed(FPS_FAST);
            m1 = sc.frame(1 - JANKY_WEBCAM_CONSTANT);
            m2 = sc.frame(2 - JANKY_WEBCAM_CONSTANT);

            if(m1 == NULL || m2 == NULL) {
                MAIN_FLAG_doflip = false;
                MAIN_FLAG_capture = false;
            }
        }

        /*
         *      MAIN_FLAG_capture
         *      capture images for calibration purposes
         */
        if(MAIN_FLAG_capture) {
            //calibrate on the resized pictures, write them to disk and append them to the YML file
            imwrite(dump_path + to_string(MAIN_FLAG_capcount) + "_A.jpg", *m1);
            imwrite(dump_path + to_string(MAIN_FLAG_capcount) + "_B.jpg", *m2);
            cout << "[stereostream] sequence saved to " + dump_path << endl;

            calibration_write += "\"" + dump_path + to_string(MAIN_FLAG_capcount) + "_A.jpg\"\n";
            calibration_write += "\"" + dump_path + to_string(MAIN_FLAG_capcount) + "_B.jpg\"\n";


            //negative flash-like effect
            Mat new_image = Mat::zeros(m1 -> size(), m1 -> type());
            Mat sub_mat = Mat::ones(m1 -> size(), m1 -> type())*255;
            subtract(sub_mat, *m1, new_image);
            delete m1;
            m1 = new Mat(new_image.size(), new_image.type());
            new_image.copyTo(*m1);

            new_image = Mat::zeros(m2 -> size(), m2 -> type());
            sub_mat = Mat::ones(m2 -> size(), m2 -> type())*255;
            subtract(sub_mat, *m2, new_image);
            delete m2;
            m2 = new Mat(new_image.size(), new_image.type());
            new_image.copyTo(*m2);


            //increase capture counter and set a timeout of 200ms to really make the flash effect stand out and
            //let the user know we captured something
            MAIN_FLAG_capture = false;
            timeout = 200;
            MAIN_FLAG_capcount++;
        }


        /*
         *      MAIN_FLAG_doflip
         *      to make things easier, the images are mirrored so it's easier to position the checkerboard
         */
        if(MAIN_FLAG_doflip) {

            Mat flip1, flip2;
            flip(*m1, flip1, 1);
            flip(*m2, flip2, 1);

            //display the capture count on top of the image

            putText(flip1, to_string(MAIN_FLAG_capcount), Point(150, 150), FONT_HERSHEY_SIMPLEX, 2.0, Scalar(255,255,255), 4);
            putText(flip2, to_string(MAIN_FLAG_capcount), Point(150, 150), FONT_HERSHEY_SIMPLEX, 2.0, Scalar(255,255,255), 4);
            g.set_block(flip1, 0);
            g.set_block(flip2, 1);
        }


        /*
         *      STREAMGRID
         *      Throw everything in streamgrid
         */
        if(MAIN_FLAG_STREAMGRID) {
            try {

                g.set_block(*stereo, 2);
                g.draw();
            }
            catch (Exception ex) {
                cout << "[main] EXCEPTION in streamgrid" << endl;
                tcperr_last = "STREAMGRID_EXCEPTION";
                cout << ex.what() << endl;
            }

        }
        else destroyWindow(STREAMGRID_TITLE);

        waitKey(timeout);

    }
    destroyAllWindows();
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




    /*      Process command line parameters     */
    if(argc > 1) {
        dump_path = argv[1];
        cout << "[main] Main directory has been set to " << argv[1] << endl;
    }
    else dump_path = argv[0];
    //add trailing slash to dump path
    dump_path = config::trailingslash(dump_path);

    /*      Load config file        */
    if(config::file_exists(dump_path + "config.yml")) {
        FileStorage fs2(dump_path + "config.yml", FileStorage::READ);
        if(fs2.isOpened()) {
            string ver = fs2["version"];
            if(ver == CAMNODE_VERSION) {
                MAIN_CALSIZE = Size(fs2["checkerboard_count_height"], fs2["checkerboard_count_width"]);
                MAIN_BLOCKSIZE = fs2["checkerboard_blocksize"];
                port = fs2["cvserver_port"];
                tcp_port = fs2["tcpctrl_port"];
                MAIN_FLAG_tcpctrl = config::to_bool(fs2["start_tcpctrl"]);
                MAIN_FLAG_nowebcams = config::to_bool(fs2["start_nowebcam"]);
                MAIN_FLAG_cvserver = config::to_bool(fs2["start_cvserver"]);
                cout << "[config] Config loaded successfully" << endl;
            }
            else {
                cout << "[config] Version mismatch! " << ver << " != " << CAMNODE_VERSION << endl;
                cout << "[config] Default config loaded." << endl;
            }
            fs2.release();
        }
        else cout << "[config] Could not open config file! Default config loaded." << endl;
    }
    else {
        cout << "[config] Config file not found! Generating new config..." << endl;
        FileStorage fs(dump_path + "config.yml", FileStorage::WRITE);
        if(fs.isOpened()) {
            fs << "checkerboard_count_height" << MAIN_CALSIZE.width;
            fs << "checkerboard_count_width" << MAIN_CALSIZE.height;
            fs << "checkerboard_blocksize" << MAIN_BLOCKSIZE;
            fs << "cvserver_port" << port;
            fs << "tcpctrl_port" << tcp_port;
            fs << "start_tcpctrl" << MAIN_FLAG_tcpctrl;
            fs << "start_nowebcam" << MAIN_FLAG_nowebcams;
            fs << "start_cvserver" << MAIN_FLAG_cvserver;
            fs << "version" << CAMNODE_VERSION;
            fs.release();
        }
        else cout << "[config] Could not create config file! Default config loaded." << endl;
    }


    //prepare camerascanner, streamgrid, stereotools
    if(MAIN_FLAG_nowebcams) JANKY_WEBCAM_CONSTANT = 0;
    camerascanner *sc = new camerascanner(MAIN_FLAG_nowebcams);
    sc -> set_tcperrno(&tcperr_last);
    streamgrid *grid = new streamgrid(2, 2, 1440, 900, STREAMGRID_TITLE);
    stereotools *st = new stereotools();

    //start cvserver if asked
    cvserver *server;
    if(MAIN_FLAG_cvserver) server = new cvserver(port);
    else server = NULL;

    //start threads
    cout << "[main] Starting threads..." << endl;
    sc -> thread_scan();
    sc -> thread_grab();

    //asynchronous main worker thread
    thread streamer(stereostream, ref(*sc), ref(*grid), ref(*st), ref(*server));


    //try and find config files for stereo camera's
    cout << "[main] Stereovision config loader finished with status " << st -> load_config(dump_path + "intrinsics.yml", dump_path + "extrinsics.yml") << endl;

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
    //string uid = "camNode:" + to_string(getrandom()) + ":" + to_string(tcp_port);
    string uid = to_string(zeromq_tools::get_first_mac());

    servus::Servus::Result r = s.announce(100, uid);
    if(r.getCode() != servus::Servus::Result::SUCCESS) {
        cerr << "Could not announce as " << uid << "!";
        cerr << "Terminating..." << endl;
        return -4;
    }
    cout << "[main] Announced as " + uid << endl;

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
                zeromq_tools::s_send(to_string(1 * MAIN_FLAG_stereo + 2 * MAIN_FLAG_doflip + 4 * MAIN_FLAG_STREAMGRID + 8 * MAIN_FLAG_getcam), tcpsocket);
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
        else if(cmd == "calibrate") {
            cout << "[calibrate] Starting in 5 seconds... get ready" << endl;
            MAIN_FLAG_capcount = 0;
            MAIN_FLAG_stereo = false;
            MAIN_FLAG_doflip = true;
            MAIN_FLAG_STREAMGRID = true;
            sleep(5);
            if(MAIN_FLAG_doflip == true) {
                cout << "[calibrate] Starting calibration sequence - will capture at intevals of 3 seconds" << endl;
                calibration_write = "<?xml version=\"1.0\"?>\n<opencv_storage>\n<imagelist>";
                for (int i = 0; i < 30; i++) {
                    cout << "[calibrate] image " << i << endl;
                    sleep(1);
                    cout << "[calibrate] 2..." << endl;
                    sleep(1);
                    cout << "[calibrate] 1..." << endl;
                    sleep(1);
                    MAIN_FLAG_capture = true;
                }
                //extra sleep so that everything up there can finish saving etc
                sleep(1);

                MAIN_FLAG_doflip = false;
                calibration_write += "</imagelist>\n</opencv_storage>";
                ofstream calibration;
                calibration.open(dump_path + "images.xml", ios::out | ios::trunc);
                calibration << calibration_write;
                calibration.close();

                cout << "[calibrate] Calibration set grabbed... calibrating cameras" << endl;
                cout << "[calibrate] Calibrating with checkerboardsize " << MAIN_CALSIZE << " and blocksize "
                     << MAIN_BLOCKSIZE << endl;


                stereo_calibration(dump_path + "images.xml", MAIN_CALSIZE, MAIN_BLOCKSIZE, dump_path, "");

                cout << "[calibrate] Calibration ended. Reloading config..." << endl;
                cout << "[calibrate] Stereovision config loader finished with status "
                     << st->load_config(dump_path + "intrinsics.yml", dump_path + "extrinsics.yml") << endl;
            }
            else {
                cout << "[calibrate] ERROR: Calibration could not be started. Did you connect 2 camera's?" << endl;
                tcperr_last = "CAL_NO_STEREO";
            }
            MAIN_FLAG_STREAMGRID = false;
        }
        else if(cmd == "recalibrate") {
            MAIN_FLAG_stereo = false;
            cout << "[calibrate] Calibrating with checkerboardsize " << MAIN_CALSIZE << " and blocksize " << MAIN_BLOCKSIZE << endl;
            stereo_calibration(dump_path + "images.xml", MAIN_CALSIZE, MAIN_BLOCKSIZE, dump_path, "");
            cout << "[calibrate] Calibration ended. Reloading config..." << endl;
            cout << "[calibrate] Stereovision config loader finished with status " << st -> load_config(dump_path + "intrinsics.yml", dump_path + "extrinsics.yml") << endl;

        }
        else if(cmd == "stereo") {
            MAIN_FLAG_stereo = !MAIN_FLAG_stereo;
            MAIN_FLAG_getcam = !MAIN_FLAG_stereo;
            if(MAIN_FLAG_stereo) cout << "[main] Stereostream enabled" << endl;
            else cout << "[main] Stereostream disabled" << endl;
        }
        else if(cmd == "stereoswap") {
            MAIN_FLAG_SWAP = !MAIN_FLAG_SWAP;
            cout << "Swap set to " << MAIN_FLAG_SWAP << endl;
        }
        else if(cmd == "cvreset") {
            destroyAllWindows();
            cout << "OpenCV windows have been reset" << endl;
        }
        else if(cmd == "cvsrestart") {
            if(MAIN_FLAG_cvserver) server -> reset();
            else cout << "[main] cvserver disabled, change program arguments and restart" << endl;
        }
        else if(cmd == "scanfast") {
            sc -> set_grab_speed(FPS_FAST);
        }
        else if(cmd == "scanmed") {
            sc -> set_grab_speed(FPS_MED);
        }
        else if(cmd == "scanslow") {
            sc -> set_grab_speed(FPS_SLOW);
        }
        else if(cmd == "cam") {
            MAIN_FLAG_getcam = !MAIN_FLAG_getcam;
            MAIN_FLAG_stereo = !MAIN_FLAG_getcam;
            if(MAIN_FLAG_getcam) cout << "[main] switched to standard webcam" << endl;
            else cout << "[main] switched to stereovision" << endl;
        }
        else if(cmd == "streamgrid") {
            MAIN_FLAG_STREAMGRID = !MAIN_FLAG_STREAMGRID;
            if(MAIN_FLAG_STREAMGRID) cout << "[main] Streamgrid enabled." << endl;
            else cout << "[main] Streamgrid disabled." << endl;
        }
        else if(cmd == "help") {
            cout << endl << "*** camNode command list ***" << endl;
            cout << "calibrate" << ": " << "start calibration setup" << endl;
            cout << "recalibrate" << ": " << "run calibration again on existing images" << endl;
            cout << "stereo" << ": " << "start live depth map generation" << endl;
            cout << "stereoswap" << ": " << "swap left and right camera in depth map generation" << endl;
            cout << "cvreset" << ": " << "destroy all OpenCV windows" << endl;
            cout << "streamgrid" << ": " << "toggle streamgrid" << endl;
            cout << "cvsrestart" << ": " << "kick all clients and restart cvserver" << endl;
            cout << "scanfast" << ": " << "set grabber speed to max (increases CPU load)" << endl;
            cout << "scanmed" << ": " << "set grabber speed to med (increases CPU load)" << endl;
            cout << "scanslow" << ": " << "set grabber speed to min (decreases CPU load)" << endl;
            cout << "cam" << ": " << "stream camera to cvserver" << endl;
            cout << "help" << ": " << "display command list" << endl;
            cout << "exit" << ": " << "quit application" << endl << endl;
        }
        else if(cmd == "nop") {
            //nope
        }
        else {
            cout << "Unknown command: '" << cmd << "'. Type 'help' for a command list." << endl;
            tcperr_last = "CMD_UNKNOWN";
        }

    }

    //drop everything, close up shop and go home.
    MAIN_FLAG_srun = false;
    MAIN_FLAG_stereo = false;
    sc -> thread_scan_join();
    sc -> thread_grab_join();
    streamer.join();

    if(MAIN_FLAG_cvserver) server -> close_conn();

    cout << "[main] *** goodbye ***" << endl;
    return 0;

}

