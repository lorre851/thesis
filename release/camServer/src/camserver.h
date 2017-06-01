//
// Created by lorre851 on 08.04.17.
//

#ifndef CAMSERVER_CAMSERVER_H
#define CAMSERVER_CAMSERVER_H

#include <iostream>
#include <vector>
#include <thread>
#include <sstream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <future>
#include "zeromq_tools.h"
#include "servus/servus.h"
#include "depthpeopledetector.h"
#include "interfaces.h"
#include "config.h"

/*
 *      STATES
 *
 *      ready       waiting for trainside init
 *      init        initializing camServer
 *      wait        halted, waiting for trainside depart
 *      results     departed, waiting for master to collect results
 *      reset       resetting
 *
 */

#define STATE_READY "ready"
#define STATE_INITIALIZING "init"
#define STATE_WAIT "wait"
#define STATE_DETECTING "detecting"
#define STATE_RESULTS "results"
#define STATE_RESET "reset"

/*
 *      CAMSERVER REPORT ERROR CODES
 */

#define ERR_OK 0
#define ERR_CAMNODE_LOST 1
#define ERR_CAMSERVER_LOST 2
#define ERR_SYNC 3
#define ERR_MASTER 4
#define ERR_TERMINATE 5
#define ERR_NO_SERVER 6
#define ERR_RESULTS_LOST 7
#define ERR_STATE 8
#define ERR_CAMNODE 9
#define ERR_CAMNODE_CRIT 10
#define ERR_END 20




/*
 *      MAIN CAMSERVER SOFTWARE
 *
 *      At boot, scan for other masters and compare their ID's to yours
 *      scan a few times just to be sure we have all servers
 *
 *      If we're in master mode, we will send commands around to the others
 *      If we're in slave mode, we will keep listening and waiting for instructions
 *
 *      Slave mode operations will continue at all times!
 *
 */
class camserver {
    private:
        //global variables
        int port;                           //own port
        long identifier;                     //own identifier
        servus::Servus *s;                  //zeroconf object for camServer announcement / discovery
        servus::Servus *cam;                //zeroconf object for camNode discovery
        std::vector<iface_node> nodes;      //own assigned camNodes
        bool force_reset = false;
        std::string state = STATE_READY;
        std::string ann = "";               //announcement name

        //TCP control channel
        zmq::context_t *context;
        zmq::socket_t *tcpsocket;

        //command control variables
        std::thread *cmd_worker = NULL;
        void cmd_loop();
        bool run_cmd = false;


        //train trip variables
        std::string IC = "";                //name of this trip (only in master mode, other camServers shouldn't care)
        std::string current_stop = "";      //name of the current stop
        unsigned long trip_id = 0;           //ID of the current trip (random unique ID generated at init)
        bool watchdog_running = false;

        //watchdog
        void watchdog();
        void report(int, std::string);
        void report(int);
        void async_report_broker(int, std::string, std::string, std::string);
        void async_push_broker(std::string, std::string, std::string);


        //configurable settings
        int cfg_gather_timeout = 5;
        int cfg_gather_retry = 5;
        int cfg_curl_timeout = 5;
        int cfg_curl_attempts = 5;
        int cfg_camnode_fails = 4;
        double cfg_camnode_fail_percentage = 0.25;
        int cfg_camserver_fails = 4;
        int cfg_master_fails = 4;
        int cfg_sync_fails = 4;
        int cfg_state_fails = 2;
        double cfg_sate_mismatch_percentage = 0.5;
        std::string cfg_server = "http://levls.be/curl.php";      //php script to push results to

        //////////////////////////////////////////////////////////////////////



        //master mode variables

        std::vector<iface_server> servers;          //list of active camServers
        std::vector<std::string> camnodes;          //list of active camNodes
        bool run_master = false;                    //thread run flag
        bool gathering = false;
        void find_nodes();
        void gather();
        void push_server();
        void reset();



        /*
         *      QUERY STORAGE FORMAT
         *
         *      <row> : timestamp - stop name - # in - # out - #camnodes_gathered - #total_camnodes
         *
         */
        std::vector< std::vector<std::string> > queries;    //vector to store trip data


    public:
        camserver(int, std::string);
        void init();
        void get_interfaces();
        void select_interface(int);
};


#endif //CAMSERVER_CAMSERVER_H
