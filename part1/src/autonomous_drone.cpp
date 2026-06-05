#include "../include/autonomous_drone.hpp"
#include "../include/drone_exceptions.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

namespace {
    std::string get_timestamp_auto() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    float calculate_distance(std::tuple<float, float, float> p1, std::tuple<float, float, float> p2) {
        float dx = std::get<0>(p1) - std::get<0>(p2);
        float dy = std::get<1>(p1) - std::get<1>(p2);
        float dz = std::get<2>(p1) - std::get<2>(p2);
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
}

AutonomousDrone::AutonomousDrone(std::string drone_name, std::tuple<float, float, float> home) {
    name = drone_name;
    home_position = home;
    ai_mode = "manual";
}

void AutonomousDrone::set_ai_mode(const std::string& mode) {
    if (mode != "manual" && mode != "auto" && mode != "return_home") {
        throw InvalidStateError("Unknown AI mode: " + mode);
    }

    ai_mode = mode;

    if (ai_mode == "return_home") {
        waypoints.insert(waypoints.begin() + current_waypoint_index, home_position);
        set_status("returning_home"); // 
    }
}

void AutonomousDrone::detect_obstacle(std::tuple<float, float, float> position, const std::string& severity) {
    auto [x, y, z] = position;
    
    std::stringstream log_entry;
    log_entry << "[" << get_timestamp_auto() << "] Obstacle at (" 
              << x << ", " << y << ", " << z << ") | Severity: " << severity;
    obstacle_log.push_back(log_entry.str());

    if (severity == "high") {
        emergency_stop();
    }
}


std::vector<std::tuple<float, float, float>> AutonomousDrone::auto_replan(const std::vector<std::tuple<float, float, float>>& obstacles) {
    std::vector<std::tuple<float, float, float>> new_waypoints;

    for (size_t i = current_waypoint_index; i < waypoints.size(); ++i) {
        auto wp = waypoints[i];
        bool path_clear = true;

        for (const auto& obs : obstacles) {
            if (calculate_distance(wp, obs) <= 5.0f) {
                std::get<2>(wp) += 10.0f; 
                path_clear = false;
                break;
            }
        }
        new_waypoints.push_back(wp);
    }

    waypoints.erase(waypoints.begin() + current_waypoint_index, waypoints.end());
    waypoints.insert(waypoints.end(), new_waypoints.begin(), new_waypoints.end());

    return new_waypoints;
}

std::string AutonomousDrone::get_info() {
    std::stringstream ss;
    ss << "AutonomousDrone | Name: " << name 
       << " | AI Mode: " << ai_mode
       << " | Battery: " << get_battery_level() << "%"
       << " | Status: " << get_status()
       << " | Progress: " << current_waypoint_index << "/" << waypoints.size();
    return ss.str();
}