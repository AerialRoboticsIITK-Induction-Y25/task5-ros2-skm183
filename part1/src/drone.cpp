#include "../include/drone.hpp"
#include "../include/drone_exceptions.hpp"
#include <sstream>

Drone::Drone() {
    altitude = 0.0f;
    max_altitude = 50.0f;
    speed = 0.0f;
}

void Drone::take_off(float target_altitude) {
    if (target_altitude > max_altitude) {
        throw AltitudeError("Altitude higher than max altitude");
    }
    altitude = target_altitude;
}

void Drone::land() {
    altitude = 0.0;
}

void Drone::emergency_stop() {
    altitude = 0.0;
    Drone::drain_battery(30.0);
}

std::string Drone::get_info() {
    std::stringstream ss;
    ss << "Drone | Name: " << name 
       << " | Battery: " << get_battery_level() << "%"
       << " | Status: " << get_status()
       << " | Altitude: " << altitude;

    return ss.str();
}