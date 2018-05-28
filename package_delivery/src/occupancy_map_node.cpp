#include "ros/ros.h"

// Standard headers
#include <chrono>
#include <string>
#include <cmath>
#include <signal.h>

// ROS message headers
#include "std_msgs/Bool.h"
#include "std_msgs/Int32.h"
#include "trajectory_msgs/MultiDOFJointTrajectory.h"

// Octomap specific headers
#include <octomap/octomap.h>
#include <octomap/OcTree.h>
#include <octomap_msgs/GetOctomap.h>
#include <octomap_msgs/conversions.h>

// MAVBench headers
#include "Drone.h"
#include "timer.h"
#include "motion_planner.h"
#include <future_collision/future_collision.h>
#include <profile_manager/profiling_data_srv.h>
#include <profile_manager/start_profiling_srv.h>

// Octomap server headers
#include <octomap_server/OctomapServer.h>

FutureCollisionChecker * fcc_ptr = nullptr;
MotionPlanner * mp_ptr = nullptr;
void sigIntHandlerPrivate(int signo) {
    if (signo == SIGINT) {
        fcc_ptr->log_data_before_shutting_down();
        mp_ptr->log_data_before_shutting_down();
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
    
    // Create an octomap server
    octomap_server::OctomapServer server;
    octomap::OcTree * octree = server.tree_ptr();

    if (nh.getParam("map_file", mapFilenameParam)) {
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

    // Create FutureCollisionChecker
    FutureCollisionChecker fcc (octree);
    fcc.setOctomapServer(&server);
    fcc_ptr = &fcc;

    // Create MotionPlanner
    MotionPlanner mp (octree);
    mp_ptr = &mp;

    while (ros::ok()) {
        ROS_INFO("Start octo");
        ros::Time start_octo = ros::Time::now();
        ros::spinOnce();
        ros::Time end_octo = ros::Time::now();
        ROS_INFO("End octo");
        // std::cout << "octomap takes " << (end_octo - start_octo).toSec() << "\n";

        ros::Time start_fcc = ros::Time::now();
        fcc.spinOnce();
        ros::Time end_fcc = ros::Time::now();
        // std::cout << "fcc takes " << (end_fcc - start_fcc).toSec() << "\n";

        ROS_INFO("Start mp");
        ros::Time start_mp = ros::Time::now();
        mp.spinOnce();
        ROS_INFO("End mp");
        ros::Time end_mp = ros::Time::now();
        // std::cout << "mp takes " << (end_mp - start_mp).toSec() << "\n";
    }
}
