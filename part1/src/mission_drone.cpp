#include "../include/mission_drone.hpp"
#include "../include/drone_exceptions.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>

namespace {
    std::string get_timestamp_md() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}

MissionDrone::MissionDrone() {
    current_waypoint_index = 0;
    mission_name = "";
}

std::tuple<float, float, float> MissionDrone::next_waypoint() {
    if (mission_complete()) {
        throw InvalidStateError("Mission is already complete. No more waypoints.");
    }

    auto current_wp = waypoints[current_waypoint_index];
    
    drain_battery(1.5f);

    visited_waypoints.push_back({current_wp, get_timestamp_md()});

    current_waypoint_index++;

    return current_wp;
}

void MissionDrone::skip_waypoint(const std::string& reason) {
    if (mission_complete()) return;

    std::string log_entry = "Skipped waypoint " + std::to_string(current_waypoint_index) + 
                            " at " + get_timestamp_md() + " | Reason: " + reason;
    
    visited_waypoints.push_back({waypoints[current_waypoint_index], "SKIPPED: " + reason});
    
    current_waypoint_index++;
}

bool MissionDrone::mission_complete() {
    return current_waypoint_index >= waypoints.size();
}

std::string MissionDrone::mission_summary() {
    std::stringstream ss;
    ss << "--- Mission Summary: " << mission_name << " ---\n";
    ss << "Waypoints Visited/Skipped:\n";
    for (const auto& log : visited_waypoints) {
        auto [x, y, z] = log.first;
        ss << "- [" << x << ", " << y << ", " << z << "] : " << log.second << "\n";
    }
    return ss.str();
}

std::string MissionDrone::get_info() {
    std::stringstream ss;
    ss << "MissionDrone | Name: " << name 
       << " | Battery: " << get_battery_level() << "%"
       << " | Status: " << get_status()
       << " | Mission: " << mission_name 
       << " | Progress: " << current_waypoint_index << "/" << waypoints.size();
    return ss.str();
}