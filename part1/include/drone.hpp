#ifndef DRONE_HPP
#define DRONE_HPP
#include "vehicle.hpp"

class Drone: public Vehicle {

    protected:
        float altitude;
        float max_altitude;

    public:
        Drone();
        
        void take_off(float target_altitude);
        void land();
        void emergency_stop();
        std::string get_info() override;

     private:
        float speed;
};

#endif