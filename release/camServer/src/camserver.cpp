//
// Created by lorre851 on 08.04.17.
//


#include "camserver.h"



/*
 *      CONSTRUCTOR
 *      Opens TCP socket and announces the camServer
 */
camserver::camserver(int port, std::string path) {
    //try making an announcement name first.
    std::random_device rd;
    std::mt19937 getrandom(rd());
    identifier = zeromq_tools::get_first_mac();
    if(identifier < 0) identifier = getrandom();

    //announce as camserver
    ann = std::to_string(identifier);

    //prepare or load config.yml
    std::cout << "[config] Reading " << path + "config.yml" << std::endl;
    if(config::file_exists(path + "config.yml")) {
        cv::FileStorage fs2(path + "config.yml", cv::FileStorage::READ);
        if(fs2.isOpened()) {
            cfg_gather_timeout = fs2["gather_timeout"];
            cfg_gather_retry = fs2["gather_retry"];
            cfg_curl_timeout = fs2["curl_timeout"];
            cfg_curl_attempts = fs2["curl_attempts"];
            cfg_camnode_fails = fs2["camnode_fails"];
            cfg_camnode_fail_percentage = fs2["camnode_fail_percentage"];
            cfg_camserver_fails = fs2["camserver_fails"];
            cfg_master_fails = fs2["master_fails"];
            cfg_sync_fails = fs2["sync_fails"];
            cfg_state_fails = fs2["state_fails"];
            cfg_sate_mismatch_percentage = fs2["sate_mismatch_percentage"];
            std::string cfg_server = fs2["server"];
            fs2.release();
            std::cout << "[config] Configuration loaded succesfully!" << std::endl;
        }
        else cout << "[config] Could not open config file! Default config loaded." << endl;
    }
    else {
        cout << "[config] Config file not found! Generating new config..." << endl;
        cv::FileStorage fs(path + "config.yml", cv::FileStorage::WRITE);
        if(fs.isOpened()) {
            fs << "gather_timeout" << cfg_gather_timeout;
            fs << "gather_retry" << cfg_gather_retry;
            fs << "curl_timeout" << cfg_curl_timeout;
            fs << "curl_attempts" << cfg_curl_attempts;
            fs << "camnode_fails" << cfg_camnode_fails;
            fs << "camnode_fail_percentage" << cfg_camnode_fail_percentage;
            fs << "camserver_fails" << cfg_camserver_fails;
            fs << "master_fails" << cfg_master_fails;
            fs << "sync_fails" << cfg_sync_fails;
            fs << "state_fails" << cfg_state_fails;
            fs << "sate_mismatch_percentage" << cfg_sate_mismatch_percentage;
            fs << "server" << cfg_server;
            fs.release();
        }
        else cout << "[config] Could not create config file! Default config loaded." << endl;
    }


    //if no port was specified, generate a random port for TCP control channel
    if(port == 0) this -> port = 1000 + (unsigned int)getrandom() % 10000;
    else this -> port = port;


    //start announcement
    s = new servus::Servus("_camserver._tcp");
    s -> set("tcp_port", std::to_string(this -> port));
    servus::Servus::Result res = s -> announce(100, ann);

    //announcement can fail if there are two instances of camServer running on the same machine
    //the second one will get a random integer as announcement ID
    if(res.getCode() != servus::Servus::Result::SUCCESS) {
        std::cerr << "[camsever] Warning! Could not announce as " << identifier << std::endl;
        delete s;
        s = new servus::Servus("_camserver._tcp");
        identifier = getrandom() % 100000;
        ann = std::to_string(identifier);
        s -> set("tcp_port", std::to_string(this -> port));
        s -> announce(100, ann);
    }
    std::cout << "[camserver] Announced as " << ann << std::endl;

    //prepare a camNode scanner
    cam = new servus::Servus("_camnode._tcp");

    //open TCP command interface
    try {
        context = new zmq::context_t(1);
        tcpsocket = new zmq::socket_t(*context, ZMQ_REP);
        tcpsocket->bind("tcp://*:" + std::to_string(this->port));
        std::cout << "[camserver] TCP control channel started on port " << this->port << std::endl;

        //start command parser
        run_cmd = true;
        cmd_worker = new std::thread(&camserver::cmd_loop, this);
    }
    catch(zmq::error_t e) {
        std::cerr << "[camserver] Failed to bind ZeroMQ socket!" << std::endl;
        std::cerr << "[camserver] Exception -- " << e.what() << std::endl;
        std::cerr << "[camserver] Aborting, sorry about that. :(" << std::endl;
        std::abort();
    }
}



/*
 *      INIT
 *
 *      Start a new trip
 *      Scan for camservers
 *      Start master mode (maybe)
 */
void camserver::init() {
    if(state == STATE_READY) {
        //initial discovery of other camservers
        state = STATE_INITIALIZING;


        std::cout << "[camserver] Discovery has started..." << std::endl;
        servus::Strings camservers = s -> discover(s -> IF_ALL, 1000);

        std::cout << "[camserver] Discovery finished. Found " << camservers.size() << " camserver(s)." << std::endl;



        //determination of the master camserver (lowest ID wins)
        unsigned long lowest_id = 0;
        for(int i = 0; i < camservers.size(); i++) {

            try {
                //ping and add to camServer interface list
                iface_server interface(s -> getHost(camservers[i]), stoi(s -> get(camservers[i], "tcp_port")), camservers[i]);
                if(interface.get_status() == CAMSERVER_CONNECTED) {
                    servers.push_back(interface);
                    //check if this ID was lower
                    if (lowest_id == 0) lowest_id = stol(camservers[i]);
                    else if (stol(camservers[i]) < lowest_id) lowest_id = stol(camservers[i]);
                }
            }
            catch(std::invalid_argument i) {
                //std::cerr << "[camserver] <init> Invalid camServer ID: " << camservers[i] << std::endl;
            }

        }




        //discovery done, check if you will be the master
        if(lowest_id == identifier) {

            //start master worker
            run_master = true;
            std::random_device rd;
            std::mt19937 getrandom(rd());
            trip_id = getrandom();



            //done. Find & assign nodes to other servers
            std::cout << "[camserver] *** STARTED MASTER MODE ***" << std::endl;



            //assign the trip ID to all camServers
            std::cout << "[camserver] Advertising trip ID as " << trip_id << std::endl;
            for(int i = 0; i < servers.size(); i++) servers[i].command("id " + std::to_string(trip_id));


            //find and assign nodes
            find_nodes();

            //start all watchdogs
            for(int i = 0; i < servers.size(); i++) servers[i].command("watchdog");

            //report the trip to wayside
            report(ERR_OK, IC);
        }


        state = STATE_WAIT;
    }
    else std::cerr << "[camserver] <init> Current state '" << state << "' does not allow init." << std::endl;



}

/*
 *      COMMAND LOOP
 *
 *      Parse incoming commands
 */
void camserver::cmd_loop() {
    //start listening

    std::vector<std::string> parser;
    while(run_cmd) {
        //parse command
        std::string cmd = "";
        while (cmd.length() == 0) cmd = zeromq_tools::s_recv(*tcpsocket);
        parser = iface_node::explode(cmd, ' ');
        std::string resp = "ack";
        bool global_cmd = false;

        /*
         *      Trip mode commands
         */

        if(state != STATE_READY) {

            //assign camnode to camserver
            if(parser[0] == "add") {
                if(parser.size() != 4) resp = "invalid";
                else {
                    //make camNode object and validate connection.
                    //expects host, port, ZeroConf ID
                    iface_node c(parser[1], stoi(parser[2]), parser[3]);
                    if (c.get_status() == CAMNODE_CONNECTED) {
                        std::cout << "[assigned] " << parser[3] << std::endl;
                        nodes.push_back(c);
                    }
                }
            }
            else if(parser[0] == "id" && parser.size() == 2 && trip_id == 0) {
                trip_id = stol(parser[1]);
                std::cout << "[camserver] Trip ID is " << trip_id << std::endl;
            }

            //we stopped somewhere, start passenger detection on nodes (make peopledetector object)
            else if(parser[0] == "halt") {
                if(state != STATE_WAIT) resp = "invalid_state";
                else if(parser.size() != 2) resp = "argument";
                else {
                    std::cout << "[camserver] Starting detection..." << std::endl;
                    current_stop = parser[1];
                    int failed = 0;
                    for(int i = 0; i < nodes.size(); i++) {
                        if(!nodes[i].start_detection()) {
                            std::cerr << "[camserver] detection could not be started on camnode " << nodes[i].get_uuid() << std::endl;
                            failed++;
                        }
                    }
                    //report number of failed detectors to caller
                    //when results are collected by master, the number of failed detections will also be sent
                    if(failed) {
                        resp = "failed " + std::to_string(failed);
                        std::cout << "[camserver] WARNING - Detection has started on " << (nodes.size() - failed) << " / " << nodes.size() << " nodes." << std::endl;
                    }
                    else std::cout << "[camserver] Detection has started on all nodes." << std::endl;
                    state = STATE_DETECTING;
                }
            }

            //we started again, stop passenger detection
            else if(parser[0] == "depart") {
                if(state == STATE_DETECTING) {
                    for (int i = 0; i < nodes.size(); i++) nodes[i].stop_detection();
                    if (run_master) new std::thread(&camserver::gather, this);
                    std::cout << "[camserver] Detection has stopped." << std::endl;
                    state = STATE_RESULTS;
                }
                else resp = "invalid_state";
            }

            //check if camnodes are in non-detection state
            else if(parser[0] == "results") {
                if(state == STATE_RESULTS) {
                    for (int i = 0; i < nodes.size(); i++) {
                        /*
                         *      Node has to be connected
                         *      If connected, node has to have results
                         *      And node has to be NOT 'done' (detection has stopped, awaiting gathering)
                         *
                         *      If it checks out, node is still busy detecting or detector is being shut down.
                         *
                         *      Also, disabled nodes tell no tales.
                         */
                        if (nodes[i].is_enabled() &&
                            nodes[i].get_status() == CAMNODE_CONNECTED
                            && !nodes[i].results_available()
                            && nodes[i].is_detecting())
                            resp = "busy";
                    }
                }
                else resp = "invalid_state";
            }

            //return results of detection cycle (in:out), destroy peopledetector object
            else if(parser[0] == "count") {
                if(state == STATE_RESULTS) {
                    //check if all results are available
                    int failed = 0;
                    for (int i = 0; i < nodes.size(); i++) {
                        if (nodes[i].get_status() != CAMNODE_CONNECTED) {
                            //TODO: this could be a severe issue (missing camnode), handle it in some way
                        } else if (!nodes[i].results_available() && nodes[i].is_detecting()) {
                            std::cerr << "results not yet available on camnode " << i << std::endl;
                            failed++;
                        }
                    }
                    if (failed) resp = "failed " + std::to_string(failed);
                    else {
                        //gather results
                        int in = 0, out = 0, count = 0;

                        for (int i = 0; i < nodes.size(); i++) {
                            if (nodes[i].get_in() == -1 || nodes[i].get_out() == -1) {
                                std::cerr << "results for node " << i << " are not available." << std::endl;
                            } else if (nodes[i].get_status() != CAMNODE_CONNECTED) {
                                nodes[i].clear_results();       //dead camnodes tell no tales.
                            } else {
                                in += nodes[i].get_in();
                                out += nodes[i].get_out();
                                nodes[i].clear_results();
                                if (in == 0 && out == 0)
                                    std::cerr << "WARNING! No passengers detected on " << nodes[i].get_uuid()
                                              << std::endl;
                                //TODO: this could be a severe issue (camera failure?), handle it in some way
                                count++;
                            }
                        }
                        resp = std::to_string(in) + ":" + std::to_string(out) + ":" + std::to_string(count) + ":" +
                               std::to_string(nodes.size());
                        std::cout << "[camserver] Results for \"" << current_stop << "\" have been delivered."
                                  << std::endl;
                        current_stop = "";
                        state = STATE_WAIT;
                    }
                }
                else resp = "invalid_state";
            }

            //reset everything and wait for a new init().
            else if(parser[0] == "terminate") {
                if(parser.size() == 2) {
                    std::cerr << "[SEVERE] Force reset initiated." << std::endl;
                    force_reset = true;
                    new std::thread(&camserver::reset, this);
                }
                else {
                    //if we still have results, we can't terminate yet.
                    if (state != STATE_WAIT) {
                        resp = "invalid_state";
                    }
                    else {
                        if (run_master) {
                            new std::thread(&camserver::push_server, this);
                        }
                        else {
                            new std::thread(&camserver::reset, this);
                        }
                    }
                }
            }

            else if(parser[0] == "trip") {
                resp = std::to_string(trip_id);
            }
            else if(parser[0] == "current_stop") {
                if(state != STATE_DETECTING) resp = "invalid_state";
                else resp = current_stop;
            }

            else if(parser[0] == "master") {
                if(run_master) resp = "yes";
                else resp = "no";
            }
            else if(parser[0] == "watchdog") {
                if(!watchdog_running) new std::thread(&camserver::watchdog, this);
                else resp = "running";
            }
            //command not found
            else global_cmd = true;

        }


        /*
         *      Non-tripmode commands
         */

        else {
            if(parser[0] == "init") {
                if(parser.size() != 2) resp = "argument";
                else {
                    std::cout << "[camserver] Trip started as " << parser[1] << std::endl;
                    IC = parser[1];
                    new std::thread(&camserver::init, this);
                }
            }
            else global_cmd = true;

        }


        /*
         *      If the command was not parsed by one of the above, it's a global command.
         *
         */
        if(global_cmd) {
            if(parser[0] == "ping") resp = "pong";
            else if(parser[0] == "state") resp = state;
            else if(parser[0] == "camnodes") resp = std::to_string(nodes.size());
            else resp = "invalid_cmd";
        }

        zeromq_tools::s_send(resp, *tcpsocket);
    }


}

/*
 *      MASTER GATHER
 *
 *      Gather results from other camnodes
 */
void camserver::gather() {
    gathering = true;
    bool ready;
    std::string stop = current_stop;


    //we'll try this 5 times
    //each time get_status is called while camserver is in disconnected state, a reconnect will be attempted
    for(int try_count = 0; try_count < cfg_gather_retry; try_count++) {
        ready = true;
        std::cout << "[camserver] <gather> Waiting up to " << cfg_gather_timeout << " seconds before gathering results... Attempt " << try_count+1 << " of " << cfg_gather_retry << "." << std::endl;
        sleep(cfg_gather_timeout);
        int in = 0, out = 0, collected = 0, total = 0;
        for (int i = 0; i < servers.size(); i++) {
            //disabled servers tell no tales
            if(servers[i].is_enabled()) {
                if (servers[i].get_status() == CAMSERVER_CONNECTED) {
                    std::string recv = servers[i].command("results");
                    if (recv != "ack") {
                        //check if this camserver had nodes to begin with
                        recv = servers[i].command("camnodes");
                        bool disable = false;
                        try {
                            int camcount = stoi(recv);
                            if(camcount == 0) disable = true;
                        }
                        catch(invalid_argument i) {
                            //do nothing
                        }
                        if(disable) servers[i].disable();
                        else ready = false;
                    }
                }
                else if (servers[i].camnodes.size() > 0 && !servers[i].get_status() == CAMSERVER_CONNECTED) {
                    ready = false;
                    std::cerr << "[SEVERE] Failed to connect to " << servers[i].get_uuid() << std::endl;
                }
            }
        }
        if(ready) {
            for (int i = 0; i < servers.size(); i++) {

                //if a server has disconnected but has no connected camnodes, it's probably malicious or has been terminated incorrectly (ctrl + z)
                if(servers[i].get_status() == CAMSERVER_CONNECTED) {
                    std::string recv = servers[i].command("count");
                    std::vector<std::string> parser = iface_node::explode(recv, ':');
                    if (parser.size() != 4) {
                        std::cerr << "[camserver] <gather> Illegal result from camserver " << i << ": " << recv << std::endl;
                    }
                    else {
                        in += stoi(parser[0]);
                        out += stoi(parser[1]);
                        collected += stoi(parser[2]);
                        total += stoi(parser[3]);
                    }
                }

            }

            try_count = 5;
            time_t  timev;
            time(&timev);           //current timestamp
            std::vector<std::string> result;

            result.push_back(std::to_string(timev));
            result.push_back(stop);
            result.push_back(std::to_string(in));
            result.push_back(std::to_string(out));
            result.push_back(std::to_string(collected));
            result.push_back(std::to_string(total));



            queries.push_back(result);

            std::cout << "[camserver] Results saved as <" << result[0] << " - " << result[1] << " - " << result[2] << " - " << result[3] << ">" << std::endl;
        }
    }
    gathering = false;

    if(!ready) {
        std::cerr << "[SEVERE] Failed to get results. Terminating all masters." << std::endl;
        for(int i = 0; i < servers.size(); i++) servers[i].command("terminate now");
    }




}



/*
 *      FIND AND ASSIGN CAMNODES
 *
 *      Search for new nodes
 *      Resolve their host
 *      Assign it to the first camServer with the least connected nodes
 */
void camserver::find_nodes() {
    std::cout << "[camserver] <find_nodes> Searching for camNodes..." << std::endl;
    //discover camnodes in network, give them 5 seconds to respond
    servus::Strings test = cam -> discover(cam -> IF_ALL, 1000);
    //iterate over all camnodes

    for(int camno = 0; camno < test.size(); camno++) {
        //search for the server with the lowest number of connected nodes.
        int index = 0;
        int least = -1;
        for (int i = 0; i < servers.size(); i++) {

            if (servers[i].is_enabled() && servers[i].get_status() == CAMSERVER_CONNECTED && (least == -1 || servers[i].camnodes.size() < least)) {
                least = servers[i].camnodes.size();
                index = i;
            }
        }

        if(least == -1) {
            std::cerr << "[SEVERE] NO ELIGABLE CAMSERVERS! Reset called." << std::endl;
            report(ERR_NO_SERVER);
            reset();
        }
        else {
            //assign to index [add host port zeroconf_id]
            std::string resp = servers[index].command("add " + cam->getHost(test[camno].data()) + " " + cam -> get(test[camno].data(), "tcp_port") + " " + (std::string) test[camno].data());
            if (resp != "ack") {
                std::cerr << "camServer down: " << index <<  " (response: " << resp << ")" << std::endl;
                servers[index].disable();
                camno--;
            }
            else {
                //add camnode to list
                servers[index].camnodes.push_back(test[camno].data());
                camnodes.push_back(test[camno].data());
            }
        }
    }
}

/*
 *      MASTER PUSH SERVER
 *
 *      Push results from all camServers to wayside server
 */
void camserver::push_server() {
    //serialize data
    std::string data = "";
    std::string ic_code = IC;
    std::string trip_code = std::to_string(trip_id);
    for(int i = 0; i < queries.size(); i++) {
        for(int j = 0; j < queries[i].size(); j++) {
            data += queries[i][j];
            if(j != queries[i].size() - 1) data += "..";
        }
        if(i != queries.size() - 1) data += "::";
    }

    async(std::launch::async, &camserver::async_push_broker, this, data, ic_code, trip_code);
    reset();
}

void camserver::async_push_broker(std::string data, std::string ic_code, std::string trip_code) {

    //push data to wayside server
    bool success = false;
    for(int i = 0; i < cfg_curl_attempts; i++) {
        try {
            curlpp::Cleanup cleaner;
            curlpp::Easy request;

            request.setOpt(new curlpp::options::Url(cfg_server.c_str()));

            {
                // Forms takes ownership of pointers!
                curlpp::Forms formParts;
                formParts.push_back(new curlpp::FormParts::Content("data", data));
                formParts.push_back(new curlpp::FormParts::Content("ic", ic_code));
                formParts.push_back(new curlpp::FormParts::Content("trip", trip_code));

                request.setOpt(new curlpp::options::HttpPost(formParts));       //POST
            }

            // The forms have been cloned and are valid for the request, even
            // if the original forms are out of scope.
            std::ostringstream os;
            os << request;
            if(os.str() == "ok") {

                std::cout << "[camserver] <push_server> Results successfully pushed to server." << std::endl;
                success = true;
                report(ERR_END, ic_code);
            }
            else {
                std::cerr << "[camserver] <push_server> error while pushing to server: " << os.str() << std::endl;
            }
            i = 5;
        }
        catch (curlpp::LogicError &e) {
            std::cout << e.what() << std::endl;
            sleep(cfg_curl_timeout);
        }
        catch (curlpp::RuntimeError &e) {
            std::cout << e.what() << std::endl;
            sleep(cfg_curl_timeout);
        }
    }

    if(!success) std::cerr << "Failed to push results to server after " << cfg_curl_attempts << " attempts... :(" << std::endl;
}

/*
 *      Report function
 *
 *      Accepts an error_code and optionally a purpetrator
 *
 */
void camserver::report(int error_code) {
    report(error_code, "n/a");
}
void camserver::report(int error_code, std::string subject) {


    std::string trip = std::to_string(trip_id);
    std::string own_id = std::to_string(identifier);

    //clion will think this is a syntax error. Luckily, the compiler knows better.
    async(std::launch::async, &camserver::async_report_broker, this, error_code, subject, trip, own_id);
}

/*
 *      Asynchronous report broker
 *
 *      Sending a report has a synchronous and an asynchronous part
 *      This function is called automatically by the report() function
 *
 */
void camserver::async_report_broker(int error_code, std::string subject, std::string trip, std::string own_id) {
    bool success = false;

    for(int i = 0; i < cfg_curl_attempts; i++) {
        try {
            curlpp::Cleanup cleaner;
            curlpp::Easy request;

            request.setOpt(new curlpp::options::Url(cfg_server.c_str()));

            {
                // Forms takes ownership of pointers!
                curlpp::Forms formParts;
                formParts.push_back(new curlpp::FormParts::Content("report", own_id));
                formParts.push_back(new curlpp::FormParts::Content("trip", trip));
                formParts.push_back(new curlpp::FormParts::Content("subject", subject));
                formParts.push_back(new curlpp::FormParts::Content("error", std::to_string(error_code)));

                request.setOpt(new curlpp::options::HttpPost(formParts));       //POST
            }

            // The forms have been cloned and are valid for the request, even
            // if the original forms are out of scope.
            std::ostringstream os;
            os << request;
            if(os.str() == "ok") {

                success = true;
            }
            else {
                std::cerr << "[camserver] <report> Could not send report: " << os.str() << std::endl;
            }
            i = 5;
        }
        catch (curlpp::LogicError &e) {
            std::cout << e.what() << std::endl;
            sleep(cfg_curl_timeout);
        }
        catch (curlpp::RuntimeError &e) {
            std::cout << e.what() << std::endl;
            sleep(cfg_curl_timeout);
        }
    }

    if(!success) std::cerr << "[camserver] <report> Failed to push report to server after " << cfg_curl_timeout << " attempts... :(" << std::endl;
}

/*
 *      Reset
 *
 *      Clear everything and prepare for a new trip
 */
void camserver::reset() {
    if(state != STATE_RESET) {
        state = STATE_RESET;
        int rescheck = 0;
        //current_stop == <something> means a detection is still in progress or results have not been gathered yet
        while(!force_reset && current_stop != "" && rescheck++ < 5) {
            std::cout << "[camserver] <reset> Cannot terminate now - I still have results!" << std::endl;
            std::cout << "[camserver] <reset> Waiting 5 seconds to retry... Attempt " << rescheck << " of 5"
                      << std::endl;
            sleep(5);
        }
        if(current_stop != "") {
            std::cerr << "[camserver] <reset> FATAL! Master did not collect results in time. Results lost."
                      << std::endl;
            report(ERR_RESULTS_LOST);
        }

        rescheck = 0;
        while(!force_reset && gathering && rescheck++ < 5) {
            std::cout << "[camserver] <reset> Cannot terminate now - still gathering results!" << std::endl;
            std::cout << "[camserver] <reset> Waiting 10 seconds to retry... Attempt " << rescheck << " of 5"
                      << std::endl;
            sleep(10);
        }
        if (gathering) std::cerr << "[camserver] <reset> FATAL! Result gathering FAILED. Results lost." << std::endl;

        //stop runs and join threads
        IC = "";
        current_stop = "";
        trip_id = 0;
        run_master = false;
        gathering = false;
        force_reset = false;

        for(int i = 0; i < nodes.size(); i++) nodes[i].stop_detection();
        nodes.clear();
        servers.clear();
        camnodes.clear();
        queries.clear();
        std::cout << "[camserver] <reset> Terminate complete. Waiting for init." << std::endl;
        state = STATE_READY;
    }
    else std::cerr << "[camserver] <reset> A reset is already in progress!" << std::endl;

}


/*
 *          Periodically check health of the system
 *
 *          Check if all camnodes are still available
 *          Check if all camservers are still available
 *          Check if all camservers are synchronized (trip started, one master available etc)
 *
 *          If something does not check out, report to wayside server
 *          Terminate operations if neccesary
 *
 */
void camserver::watchdog() {
    if(!watchdog_running) {
        watchdog_running = true;

        std::cout << "[watchdog] Initialized." << std::endl;

        int camnode_fails[nodes.size()];
        int camserver_fails[servers.size()];
        int sync_failures = 0, master_failures = 0, state_failures = 0;

        for (int i = 0; i < nodes.size(); i++) camnode_fails[i] = 0;
        for (int i = 0; i < servers.size(); i++) camserver_fails[i] = 0;


        while (state != STATE_READY && state != STATE_RESET) {
            bool severe_failure = false;
            int masters = 0;
            int failed_nodes = 0;
            int state_mismatches = 0;

            std::map<std::string, int> states;
            std::map<std::string, int> stops;

            //ping all my camnodes
            for (int i = 0; i < nodes.size(); i++) {
                if (nodes[i].is_enabled() && nodes[i].get_status() == CAMNODE_CONNECTED) {
                    //check for critical errors
                    std::string cs = nodes[i].command("errno");
                    if (cs == "NO_CAL" || cs == "CAL_NO_STEREO" || cs == "CAMSCAN_NO_STEREO") {
                        //disable camNode
                        nodes[i].disable();
                        nodes[i].stop_detection();
                        std::cerr << "[watchdog] Disabled camNode " << nodes[i].get_uuid() << ", got error " << cs
                                  << std::endl;
                        //report critical issue
                        report(ERR_CAMNODE_CRIT, nodes[i].get_uuid() + ":" + cs);
                    } else if (cs != "ok" && cs != "CMD_UNKNOWN") {
                        std::cerr << "[watchdog] Warning from camNode " << nodes[i].get_uuid() << ", got error " << cs
                                  << std::endl;
                        //report non critical issue
                        report(ERR_CAMNODE, nodes[i].get_uuid() + ":" + cs);
                    }
                    camnode_fails[i] = 0;
                } else camnode_fails[i]++;
            }

            //ping all my camservers, check their trip synchronisation
            for (int i = 0; i < servers.size(); i++) {
                if (servers[i].get_status() == CAMSERVER_CONNECTED) {
                    //connected, reset failed attempts
                    camserver_fails[i] = 0;

                    //validate curren trip
                    if (servers[i].command("trip") != std::to_string(trip_id)) sync_failures++;
                    else sync_failures = 0;

                    //validate number of masters
                    if (servers[i].command("master") == "yes") masters++;

                    //validate state
                    std::string server_state = servers[i].command("state");
                    if (server_state != state) {
                        state_mismatches++;
                    }
                    std::map<std::string, int>::iterator it = states.find(server_state);
                    if (it != states.end()) it->second += 1;       //if found, add one
                    else states[server_state] = 1;                  //else f* it


                    //get current stop
                    std::string server_stop = servers[i].command("current_stop");
                    std::map<std::string, int>::iterator jt = stops.find(server_stop);
                    if (jt != stops.end()) it->second += 1;       //if found, add one
                    else stops[server_stop] = 1;                  //else f* it

                } else if (servers[i].is_enabled()) camserver_fails[i]++;
            };
            if (masters != 1) master_failures++;


            //if > 75% of other camServers have a diffrent state from mine, count it as a state failure
            if ((1.0 * state_mismatches) / (1.0 * servers.size()) > cfg_sate_mismatch_percentage) state_failures++;
            else state_failures = 0;

            /*          Validate results        */


            //check number of failed nodes
            for (int i = 0; i < nodes.size(); i++) {
                if (camnode_fails[i] > cfg_camnode_fails) {
                    report(ERR_CAMNODE_LOST, nodes[i].get_uuid());
                    failed_nodes++;
                }
            }

            //if not enough nodes are available, terminate
            if (failed_nodes > 0 && (1.0 * failed_nodes) / (1.0 * nodes.size()) >= cfg_camnode_fail_percentage) {
                std::cerr << "[watchdog] Not enough camnodes available to continue." << std::endl;
                severe_failure = true;
            }


            //if a server has failed multiple times, terminate
            for (int j = 0; j < servers.size(); j++) {
                if (camserver_fails[j] > cfg_camserver_fails) {
                    //check if this camServer mattered anyways
                    std::string poll = servers[j].command("camnodes");
                    bool disable = false;
                    try {
                        int count = stoi(poll);
                        if (count == 0) disable = true;
                    }
                    catch (invalid_argument i) {

                    }
                    if (disable) {
                        camserver_fails[j] = 0;
                        servers[j].disable();
                        std::cerr << "[watchdog] Disabled server " << servers[j].get_uuid() << std::endl;
                    } else {
                        report(ERR_CAMSERVER_LOST, servers[j].get_uuid());
                        std::cerr << "[watchdog] A camserver has been permanently lost." << std::endl;
                        severe_failure = true;
                    }

                }
            }



            //if not all trip ID's are synchronized, terminate
            if (sync_failures > cfg_sync_fails) {
                report(ERR_SYNC);
                std::cerr << "[watchdog] Sync failure, all camserves should be tripping right now" << std::endl;
                severe_failure = true;
            }

            //if there's an unexpected number of masters on the network, terminate
            if (master_failures > cfg_master_fails) {
                report(ERR_MASTER);
                std::cerr << "[watchdog] Master issue, expected one master on network, got " << masters << std::endl;
                severe_failure = true;
            }


            //if there's a state mismatch issue, try and resync or terminate.
            if (state_failures > cfg_state_fails) {
                //determine the state of other camservers
                std::map<std::string, int>::iterator it = states.begin(), end = states.end();

                int max = 0;
                std::string prevelant_state = "";
                while (it != end) {
                    if (it->second > max) {
                        max = it->second;
                        prevelant_state = it->first;
                    }
                    it++;
                }

                /*
                 *      Impossible states:
                 *      - Everyone is waiting but I still have results
                 *      - Everyone is resetting but I'm doing fuck knows what
                 *
                 */
                bool recovered = false;

                //Case 1: everyone is detecting but I'm still waiting -> go to STATE_DETECT
                if (prevelant_state == STATE_DETECTING && state == STATE_WAIT) {
                    for (int i = 0; i < servers.size(); i++) {
                        if (servers[i].get_uuid() == ann) {
                            //get prevelant stop
                            std::map<std::string, int>::iterator it = stops.begin(), end = stops.end();
                            int max = 0;
                            std::string prevelant_stop;
                            while (it != end) {
                                if (it->second > max) {
                                    max = it->second;
                                    prevelant_stop = it->first;
                                }
                                it++;
                            }
                            std::cout << "[watchdog] Pushed camServer to STATE_DETECT" << std::endl;
                            if (servers[i].command("halt " + prevelant_stop) == "ack") recovered = true;
                            i = servers.size();
                        }
                    }
                }

                    //Case 2: everyone has results but I'm still detecting -> go to STATE_RESULTS
                else if (prevelant_state == STATE_RESULTS && state == STATE_DETECTING) {
                    for (int i = 0; i < servers.size(); i++) {
                        if (servers[i].get_uuid() == ann) {
                            std::cout << "[watchdog] Pushed camServer to STATE_RESULTS" << std::endl;
                            if (servers[i].command("depart") == "ack") recovered = true;;
                            i = servers.size();

                        }
                    }
                }

                    //Case 3: something has gone wrong terribly.
                else {
                    std::cerr << "[watchdog] Unrecoverable state mismatch error." << std::endl;
                    std::cerr << "[watchdog] Network has '" << prevelant_state << "', I have '" << state << "'"
                              << std::endl;
                    report(ERR_STATE);
                }

                if (!recovered) severe_failure = true;

            }

            //if one of the checks triggered a severe failure, terminate
            if (severe_failure) {
                report(ERR_TERMINATE);
                std::cerr << "[SEVERE] Terminated by watchdog." << std::endl;
                force_reset = true;
                reset();
                break;
            } else sleep(5);
        }
        watchdog_running = false;
    }
    else std::cout << "[watchdog] Antoher watchdog is already running" << std::endl;
}


void camserver::get_interfaces() {
    std::cout << "-- interface list --" << std::endl;
    for(int i = 0; i < nodes.size(); i++) {
        std::cout << "[" << i << "] " << nodes[i].get_uuid() << std::endl;
    }
    std::cout << "-- end of list --" << std::endl;
}
void camserver::select_interface(int interface) {
    if(interface >= 0 && interface < nodes.size() || interface == -1) {
        for(int i = 0; i < nodes.size(); i++) nodes[i].visualize = false;
        if(!nodes[interface].is_enabled()) std::cerr << "this node is broken and cannot be visualised." << std::endl;
        else if(interface != -1) nodes[interface].visualize = true;
    }
    else std::cerr << "invalid interface: " << interface << std::endl;
}
