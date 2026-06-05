#include <iostream>
#include <vector>
#include <tuple>

#include "../include/drone_exceptions.hpp"
#include "../include/vehicle.hpp"
#include "../include/drone.hpp"
#include "../include/mission_drone.hpp"
#include "../include/autonomous_drone.hpp"

int main() {
    std::cout << "=== ARIITK Drone Fleet Manager: C++ OOP Test ===\n\n";

    Drone basic_drone;
    basic_drone.name = "Basic-Beta";
    
    MissionDrone mission_drone;
    mission_drone.name = "Mission-Gamma";
    
    AutonomousDrone auto_drone("Alpha-Auto", {0.0f, 0.0f, 0.0f});
    
    auto_drone.waypoints = {
        {10.0f, 0.0f, 5.0f}, 
        {20.0f, 10.0f, 5.0f}, 
        {30.0f, 0.0f, 5.0f}
    };

    std::cout << "--- Polymorphism Test ---\n";
    std::vector<Vehicle*> fleet;
    fleet.push_back(&basic_drone);
    fleet.push_back(&mission_drone);
    fleet.push_back(&auto_drone);

    for (Vehicle* v : fleet) {
        std::cout << v->get_info() << "\n";
    }
    std::cout << "\n";

    std::cout << "--- Exception Handling Test ---\n";
    try {
        std::cout << "Attempting invalid status update...\n";
        auto_drone.set_status("dancing"); 
    } catch (const InvalidStateError& e) {
        std::cout << "Caught Expected Error: " << e.what() << "\n";
    } catch (const DroneException& e) {
        std::cout << "Caught Base Drone Error: " << e.what() << "\n";
    }

    try {
        std::cout << "Attempting to drain empty battery...\n";
        auto_drone.drain_battery(100.0f); 
        auto_drone.drain_battery(10.0f);
    } catch (const BatteryDepletedError& e) {
        std::cout << "Caught Expected Error: " << e.what() << "\n";
    }

    try {
        auto_drone.set_status("charging");
        auto_drone.charge_battery(100.0f, 60);
        auto_drone.set_status("idle");
    } catch (...) {
        std::cout << "Failed to recharge for mission.\n";
    }
    std::cout << "\n";

    std::cout << "--- Full Autonomous Mission Test ---\n";
    try {
        auto_drone.set_status("flying");
        std::cout << "Alpha-Auto Taking off...\n";
        

        auto_drone.take_off(5.0f); 

        while (!auto_drone.mission_complete()) {
            auto current_wp = auto_drone.next_waypoint();
            std::cout << "Reached waypoint: [" 
                      << std::get<0>(current_wp) << ", " 
                      << std::get<1>(current_wp) << ", " 
                      << std::get<2>(current_wp) << "]\n";


            if (std::get<0>(current_wp) == 20.0f) {
                std::cout << "! OBSTACLE DETECTED !\n";
                auto_drone.detect_obstacle({25.0f, 10.0f, 5.0f}, "high");
            }
        }
    } catch (const DroneException& e) {
        std::cout << "Mission Interrupted: " << e.what() << "\n";
    }

    std::cout << "\n" << auto_drone.mission_summary() << "\n";

    return 0;
}