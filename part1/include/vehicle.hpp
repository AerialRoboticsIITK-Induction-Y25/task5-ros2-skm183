#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <string>
#include <vector>

class Vehicle {
    public:
        Vehicle();
        std::string name;

        virtual std::string get_info() = 0;
        void drain_battery(float amount);
        void charge_battery(float amount, int duration_seconds);
        bool is_critical();
        std::string get_flight_log();
        float get_battery_level();
        std::string get_status();

        void set_status(std::string new_status);

     private:
        float battery_level;
        std::string status;
        std::vector<std::string> flight_log;

};

#endif