#include "../include/vehicle.hpp"
#include "../include/drone_exceptions.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>


namespace {
    std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}

Vehicle::Vehicle() {
    battery_level = 100.0f;
    status = "idle";
}

float Vehicle::get_battery_level() {
    return battery_level;
}

std::string Vehicle::get_status() {
    return status;
}

void Vehicle::set_status(std::string new_status) {
    if (new_status != "idle" && new_status != "flying" && new_status != "charging") {
        std::cout << "Status must be idle, flying, or charging.";
    }
    
    status = new_status;

    std::string log_entry = "[" + get_current_timestamp() + "] Status updated to: " + status;
    flight_log.push_back(log_entry);
}

void Vehicle::drain_battery(float amount) {
    if (battery_level <= 0) {
        throw BatteryDepletedError("Battery empty");
    }
    battery_level -= amount;
    if (battery_level < 0) {
        battery_level = 0.0;
    }
}

void Vehicle::charge_battery(float amount, int duration_seconds) {
    if (status != "charging") {
        throw InvalidStateError("Status not set to charging");
    }
    if (battery_level >= 100) {
        std::cout << "Battery full";
    }
    battery_level += amount;
    if (battery_level > 100) {
        battery_level = 100.0;
    }
}

bool Vehicle::is_critical() {
    if (battery_level <= 20) return true;
    else return false;
}
