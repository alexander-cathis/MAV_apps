#include <Drone.h>
#include <future_collision/future_collision.h>
#include <octomap_server/OctomapServer.h>
#include <ros/init.h>
#include <ros/node_handle.h>
#include <ros/param.h>
#include <rosconsole/macros_generated.h>
#include <signal.h>
#include <XmlRpcValue.h>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "motion_planner.h"

/*
void log_data_before_shutting_down()
{
	std::string ns = ros::this_node::getName();
    profile_manager::profiling_data_srv profiling_data_srv_inst;
    profile_manager::profiling_data_verbose_srv profiling_data_verbose_srv_inst;
    server->profiling_container.setStatsAndClear();

	for (auto &data: server->profiling_container.container){
    	profiling_data_srv_inst.request.key = data.data_key_name + " last window's avg: ";
		vector<double>* avg_stat = data.getStat("avg");
		if (avg_stat) {
			profiling_data_srv_inst.request.value = avg_stat->back();

		}
		else{
			profiling_data_srv_inst.request.value = nan("");
		}
		profile_manager.clientCall(profiling_data_srv_inst);
	}

    profiling_data_verbose_srv_inst.request.key = ros::this_node::getName()+"_verbose_data";
	profiling_data_verbose_srv_inst.request.value = "\n" + server->profiling_container.getStatsInString();
    profile_manager.verboseClientCall(profiling_data_verbose_srv_inst);

*/

void sigIntHandlerPrivate(int signo) {
    if (signo == SIGINT) {
        //fcc_ptr->log_data_before_shutting_down();
        //mp_ptr->log_data_before_shutting_down();
        ros::shutdown();
    }
    exit(0);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "occupancy_map_node");
    ros::NodeHandle nh("~");
    std::string mapFilename(""), mapFilenameParam("");
    signal(SIGINT, sigIntHandlerPrivate);

    //----------------------------------------------------------------- 
	// *** F:DN variables	
	//----------------------------------------------------------------- 
    
    // Create a Drone object
    std::string ip_addr, localization_method;
    ros::param::get("/ip_addr", ip_addr);
    uint16_t port = 41451;
    if(!ros::param::get("/localization_method", localization_method)) {
        ROS_FATAL("Could not start occupancy map node. Localization parameter missing!");
        exit(-1);
    }
    Drone drone(ip_addr.c_str(), port, localization_method);
    
    // Create an octomap server
    octomap_server::OctomapServer server;
    octomap::OcTree * octree = server.tree_ptr();


    if (nh.getParam("/occupancy_map_node/map_file", mapFilenameParam)) {
        if (mapFilename != "") {
            ROS_WARN("map_file is specified by the argument '%s' and rosparam '%s'. now loads '%s'", mapFilename.c_str(), mapFilenameParam.c_str(), mapFilename.c_str());
        } else {
            mapFilename = mapFilenameParam;
        }
    }
    if (mapFilename != "") {
        if (!server.openFile(mapFilename)){
            ROS_ERROR("Could not open file %s", mapFilename.c_str());
            exit(1);
        }
    }

    while (ros::ok()) {
        ros::spinOnce();
    }
}

