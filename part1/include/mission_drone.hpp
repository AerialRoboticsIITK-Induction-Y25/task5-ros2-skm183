#ifndef MISSIONDRONE_HPP
#define MISSIONDRONE_HPP
#include "drone.hpp"

#include <string>
#include <vector>

class MissionDrone: public Drone {

    public:
        MissionDrone();

        std::string mission_name;
        std::vector<std::tuple<float, float, float>> waypoints;
        int current_waypoint_index;

        std::tuple<float, float, float> next_waypoint();
        void skip_waypoint(const std::string& reason);
        bool mission_complete();
        std::string mission_summary();
        std::string get_info() override;

     private:
        std::vector<std::pair<std::tuple<float,float,float>, std::string>> visited_waypoints;
};

#endif