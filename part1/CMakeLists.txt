cmake_minimum_required(VERSION 3.8)
project(drone_fleet)

include_directories(headers)

add_executable(fleet_test 
    src/main.cpp 
    src/Vehicle.cpp 
    src/Drone.cpp 
    src/MissionDrone.cpp 
    src/AutonomousDrone.cpp
)

target_compile_options(fleet_test PRIVATE -Wall -Wextra)