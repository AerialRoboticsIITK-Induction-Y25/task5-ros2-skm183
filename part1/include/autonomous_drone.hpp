#ifndef AUTONOMOUSDRONE_HPP
#define AUTONOMOUSDRONE_HPP

#include "mission_drone.hpp"
#include <string>
#include <vector>
#include <tuple>

class AutonomousDrone : public MissionDrone {
public:
    AutonomousDrone(std::string drone_name, std::tuple<float, float, float> home);

    void set_ai_mode(const std::string& mode);
    void detect_obstacle(std::tuple<float, float, float> position, const std::string& severity);
    std::vector<std::tuple<float, float, float>> auto_replan(const std::vector<std::tuple<float, float, float>>& obstacles);

    std::string get_info() override;

private:
    std::string ai_mode;
    std::tuple<float, float, float> home_position;
    std::vector<std::string> obstacle_log;
};

#endif