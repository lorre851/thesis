//
// Created by lorre851 on 03.12.16.
//

#include "camerascanner.h"

/******************************************
 *
 *      	PRIVATE FUNCTIONS
 *
 ******************************************/


//Converts u8 to string
string camerascanner::u8_string(__u8 *source) {
    int it = 0;
    string res;
    while(source[it]) {
        res += source[it++];
    }
    return res;
}

//Check if a camera ID is valid
//in other words, non negative and smaller than the maximum scan count
bool camerascanner::is_valid(int cam_id) {
    return cam_id >= lowest_cam && cam_id < CAMERA_COUNT;
}

//Safely allocate a camera
int camerascanner::allocate(int id, v4l2_capability &cap) {
    if(is_valid(id)) {
        if(!cam_status[id]) {
            cout << "[camerascanner] <allocate> allocating camera " << id << endl;

            properties[id].name = u8_string(cap.card);
            properties[id].identifier = u8_string(cap.bus_info);
            properties[id].width = 640;
            properties[id].height = 360;
            //TODO: dynamically allocate width and height

            camera[id] = new VideoCapture(id);                  //first allocate a videocapture object
            camera[id] -> set(CV_CAP_PROP_FRAME_WIDTH, properties[id].width);
            camera[id] -> set(CV_CAP_PROP_FRAME_HEIGHT, properties[id].height);
            cam_status[id] = true;                              //then tell the rest of the system it's OK to use it
            cout << "[camerascanner] <allocate> allocation successful. ID: " << properties[id].name << "_" << properties[id].identifier << endl;
        }
        return 0;
    }
    else return ILLEGAL_CAM_ID;
}

//SAFELY (yes, the safely is capitalized here) deallocate a camera
int camerascanner::deallocate(int id) {
    if(is_valid(id)) {
        if(cam_status[id]) {
            cout << "[camerascanner] <deallocate> brutally murdering threads..." << endl;    //If the grabber is active, kill it before it attempts to lay eggs and multiply.
            bool restart = thread_grab_join();                  //or worse - attempts to call a member function of a NULL pointer
            while(locked[id]);                                  //wait until buffer is released
            locked[id] = true;
            cout << "[camerascanner] <deallocate> deallocating camera " << id << endl;
            cam_status[id] = false;                             //Deallocate camera & set object to NULL
            if (camera[id] != NULL) {
                delete camera[id];
                camera[id] = NULL;
            }
            if(frames[id] != NULL) {
                delete frames[id];
                frames[id] = NULL;
            }
            cout << "[camerascanner] <deallocate> camera disconnected succesfully" << endl;
            locked[id] = false;
            if(restart) thread_grab();                         //if the grabber was active, restart it. THE GRABBER; NOT THE SCANNER I-DI-OT
        }
        return 0;
    }
    else return ILLEGAL_CAM_ID;
}


/*
 *      Public functions
 */

//constructor for initiaiting private and public variables
camerascanner::camerascanner(bool disable_cam0) {
    if(disable_cam0) {
        lowest_cam = 1;
        cout << "[camerascanner] Starting with video0 disabled (gets rid of internal webcam)" << endl;
    }
    for(int i = 0; i < CAMERA_COUNT; i++) {
        cam_status[i] = false;
        camera[i] = NULL;
        locked[i] = false;
        frames[i] = NULL;
        cooldown[i] = 0;
    }

    camerascanner::scan(0);
    cout << "[camerascanner] Ready." << endl;
}

//scan for connected cameras and update private variables
void camerascanner::scan(int timeout) {
    int fd;
    string index;
    struct v4l2_capability cap;
    bool run = true;
    while(run) {
        for (int camera_count = 0; camera_count < CAMERA_COUNT; camera_count++) {
            index = "/dev/video" + to_string(camera_count);
            if ((fd = open(index.c_str(), O_RDWR)) < 0) {
                deallocate(camera_count);
            }
            else {
                if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
                    cout << "[camerascanner] <scan> ERROR: cannot query this device: " << camera_count << endl;
                    tcpstatus("CAMSCAN_ERR_ALLOCATE_DEV: video" + to_string(camera_count));
                    deallocate(camera_count);
                }
                else {
                    allocate(camera_count, cap);
                }
                close(fd);
            }

        }
        if(timeout <= 0 || thread_scan_run == false) run = false;
        else usleep(timeout);
    }
}

//grab frames from all cameras and put them in buffer
void camerascanner::grab_frames(int timeout) {
    bool grabstatus[CAMERA_COUNT];      //keep track of which cameras had a successful frame grab
    bool run = true;

    while(run) {
        //Stage 1: quick grab frames to reduce latency between frame grabs (important for stereovision!)
        for (int camcount = 0; camcount < CAMERA_COUNT; camcount++) {
            if (cam_status[camcount]) {
                try {
                    if (camera[camcount]->grab()) {
                        grabstatus[camcount] = true;
                    }
                    else {
                        cout << "[camerascanner] <grab_frames> Warning: expected frame to grab, but got nothing." << endl;
                        tcpstatus("CAMSCAN_GRAB_FAILED: video" + to_string(camcount));
                        grabstatus[camcount] = false;
                    }
                }
                catch (Exception e) {
                    grabstatus[camcount] = false;
                    tcpstatus("CAMSCAN_GRAB_EXCEPTION: video" + to_string(camcount) + ": " + e.what());
                    cout << "[camerascanner] <grab_frames> EXCEPTION IN GRAB: " << e.what() << endl;
                }
            }
        }

        //Stage 2: flush frames to global buffer
        for (int camcount = 0; camcount < CAMERA_COUNT; camcount++) {
            while(locked[camcount]); //wait until no other thread is using frame data, then lock write access
            locked[camcount] = true;
            if(frames[camcount] == NULL) frames[camcount] = new Mat();
            if (cam_status[camcount] && grabstatus[camcount]) {
                try {
                    //frame grab successful, now retrieve it
                    camera[camcount]->retrieve(*frames[camcount]);
                }
                catch (Exception e) {
                    cout << "[camerascanner] <grab_frames> EXCEPTION IN FLUSH: " << e.what() << endl;
                    tcpstatus("CAMSCAN_FLUSH_EXCEPTION: video" + to_string(camcount) + ": " + e.what());
                    delete frames[camcount];
                    frames[camcount] = NULL;
                }
            }
            else {
                delete frames[camcount];
                frames[camcount] = NULL;
            }
            locked[camcount] = false;
        }

        if(timeout <= 0 || thread_grab_run == false) run = false;
        else usleep(grabspeed);
    }
}

//safely retreive a frame from memory
Mat* camerascanner::frame(int id) {
    if(is_valid(id) && cam_status[id]) {
        if(cooldown[id] > 0) {
            cooldown[id]--;
            if(cooldown[id] == 0) cout << "[camerascanner] <frame> retrying for camera " << id << "..." << endl;
            return NULL;
        }
        else {
            while (locked[id]);
            locked[id] = true;
            if (frames[id] != NULL) {
                Mat *copy = new Mat(*frames[id]);
                locked[id] = false;
                return copy;
            }
            else {
                locked[id] = false;
                cout << "[camerascanner] <frame> WARNING: expected frame in buffer " << id << ", got NULL instead" << endl;
                cout << "[camerascanner] <frame> waiting 500 cycles before retrying..." << endl;
                tcpstatus("CAMSCAN_BUFFER_EMPTY: video" + to_string(id));
                cooldown[id] = 500;
                return NULL;
            }
        }
    }
    else return NULL;
}

//Grab a dual frame for stereovision purposes. Designed to limit latency between the two frames
Mat** camerascanner::get_stereo_frame(int id1, int id2) {
    Mat **result = new Mat*[2];
    bool thr = thread_grab_join();
    result[0] = frame(id1);
    result[1] = frame(id2);
    if(thr) thread_grab();
    if(result[0] == NULL || result[1] == NULL) {
        cout << "[camerascanner] <get_stereo_frame> ERROR: Could not fetch a stereo frame, one of the buffers was empty!" << endl;
        tcpstatus("CAMSCAN_NO_STEREO");
        return NULL;
    }
    return result;
}


//Get a hardware identifier for this camera
//this will always return a string
string camerascanner::identifier(int id) {
    if(is_valid(id) && cam_status[id]) {
        return properties[id].name + "_" + properties[id].identifier;
    }

    else return "Undefined";
}


/*
 *      THREAD FUNCTIONS
 *      Treat with extreme caution
 */

//create a camera scanner thread
thread *camerascanner::thread_scan() {
    if(!thread_scan_run) {
        thread_scan_run = true;
        scanner = new thread(&camerascanner::scan, this, 50000);
    }
    return scanner;
}

//create a frame grabber thread
thread *camerascanner::thread_grab() {
    if(!thread_grab_run) {
        thread_grab_run = true;
        grabber = new thread(&camerascanner::grab_frames, this, 100000);
    }
    return scanner;
}

//join scanner
bool camerascanner::thread_scan_join() {
    if(thread_scan_run) {
        thread_scan_run = false;
        scanner -> join();
        delete scanner;
        scanner = NULL;
        return true;
    }
    else return false;
}

//join frame grabber
bool camerascanner::thread_grab_join() {
    if(thread_grab_run) {
        thread_grab_run = false;
        grabber -> join();
        delete grabber;
        grabber = NULL;
        return true;
    }
    else return false;
}


int camerascanner::force_refresh(int id) {
    if(is_valid(id)) {
        if(cam_status[id]) {
            deallocate(id);
            return 0;
        }
        else return NOT_CONNECTED;
    }
    else return ILLEGAL_CAM_ID;
}

void camerascanner::set_grab_speed(int t_speed) {
    if(t_speed == FPS_FAST || t_speed == FPS_MED || t_speed == FPS_SLOW) grabspeed = t_speed;
    else cout << "[camerascanner] <set_grab_speed> Invalid value: " << t_speed << endl;

}

void camerascanner::set_tcperrno(string* str) {
    tcperrno = str;
}

void camerascanner::tcpstatus(string status) {
    *tcperrno = status;
}
