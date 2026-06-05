#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <chrono>

struct DroneState {
    std::string name = "Unknown";
    double battery = 0.0;
    std::string altitude = "0.0";
    std::string waypoint = "0/0";
    std::string status = "offline";
};

class FleetManagerNode : public rclcpp::Node {
public:
    FleetManagerNode() : Node("fleet_manager") {
        std::vector<std::string> drones = {"Alpha", "Beta", "Gamma"};

        for (const auto& d_name : drones) {
            std::string base_topic = "/drone/" + d_name;

            auto status_sub = this->create_subscription<std_msgs::msg::String>(
                base_topic + "/status", 10,
                [this, d_name](const std_msgs::msg::String::SharedPtr msg) {
                    this->parse_status_string(d_name, msg->data);
                });
            status_subs_.push_back(status_sub);

            auto telemetry_sub = this->create_subscription<std_msgs::msg::String>(
                base_topic + "/telemetry", 10,
                [this, d_name](const std_msgs::msg::String::SharedPtr msg) {
                    this->parse_manual_json(d_name, msg->data);
                });
            telemetry_subs_.push_back(telemetry_sub);

            auto alert_sub = this->create_subscription<std_msgs::msg::String>(
                base_topic + "/alert", 10,
                [this, d_name](const std_msgs::msg::String::SharedPtr msg) {
                    RCLCPP_WARN(this->get_logger(), "[%s] ALERT: %s", d_name.c_str(), msg->data.c_str());
                });
            alert_subs_.push_back(alert_sub);

            auto mission_sub = this->create_subscription<std_msgs::msg::String>(
                base_topic + "/mission_complete", 10,
                [this, d_name](const std_msgs::msg::String::SharedPtr msg) {
                    RCLCPP_INFO(this->get_logger(), "[%s] %s", d_name.c_str(), msg->data.c_str());
                });
            mission_subs_.push_back(mission_sub);

            fleet_state_[d_name].name = d_name;
        }

        report_timer_ = this->create_wall_timer(
            std::chrono::seconds(5), std::bind(&FleetManagerNode::print_report, this));

        trigger_srv_ = this->create_service<std_srvs::srv::Trigger>(
            "/fleet/status_report",
            [this](const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
                   std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
                (void)request; // Unused
                RCLCPP_INFO(this->get_logger(), "Manual Report Triggered via Service.");
                this->print_report();
                response->success = true;
                response->message = "Report printed to terminal.";
            });

        RCLCPP_INFO(this->get_logger(), "Fleet Manager Node Initialized.");
    }

private:
    std::map<std::string, DroneState> fleet_state_;
    
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> status_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> alert_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> mission_subs_;

    rclcpp::TimerBase::SharedPtr report_timer_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr trigger_srv_;

    void parse_status_string(const std::string& drone_name, const std::string& data) {
        std::istringstream ss(data);
        std::string token;
        
        while (std::getline(ss, token, '|')) {
            size_t delim = token.find(':');
            if (delim != std::string::npos) {
                std::string key = token.substr(0, delim);
                std::string value = token.substr(delim + 1);
                
                if (key == "altitude") fleet_state_[drone_name].altitude = value;
                if (key == "waypoint") fleet_state_[drone_name].waypoint = value;
            }
        }
    }

    void parse_manual_json(const std::string& drone_name, std::string json_str) {
        json_str.erase(std::remove(json_str.begin(), json_str.end(), '{'), json_str.end());
        json_str.erase(std::remove(json_str.begin(), json_str.end(), '}'), json_str.end());
        json_str.erase(std::remove(json_str.begin(), json_str.end(), '"'), json_str.end());
        json_str.erase(std::remove(json_str.begin(), json_str.end(), ' '), json_str.end());

        std::istringstream ss(json_str);
        std::string pair;
        
        while (std::getline(ss, pair, ',')) {
            size_t delim = pair.find(':');
            if (delim != std::string::npos) {
                std::string key = pair.substr(0, delim);
                std::string value = pair.substr(delim + 1);
                
                if (key == "battery") {
                    try {
                        fleet_state_[drone_name].battery = std::stod(value);
                    } catch (...) {}
                }
                if (key == "status") fleet_state_[drone_name].status = value;
            }
        }
    }

    void print_report() {
        std::cout << "\n====================== FLEET STATUS REPORT ======================\n";
        std::cout << std::left 
                  << std::setw(10) << "DRONE" 
                  << std::setw(15) << "BATTERY (%)" 
                  << std::setw(15) << "ALTITUDE" 
                  << std::setw(15) << "WAYPOINT" 
                  << std::setw(15) << "STATUS" << "\n";
        std::cout << "-----------------------------------------------------------------\n";
        
        // Print in a fixed order
        std::vector<std::string> order = {"Alpha", "Beta", "Gamma"};
        for (const auto& name : order) {
            auto& state = fleet_state_[name];
            std::cout << std::left 
                      << std::setw(10) << state.name 
                      << std::setw(15) << std::fixed << std::setprecision(1) << state.battery 
                      << std::setw(15) << state.altitude 
                      << std::setw(15) << state.waypoint 
                      << std::setw(15) << state.status << "\n";
        }
        std::cout << "=================================================================\n\n";
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FleetManagerNode>());
    rclcpp::shutdown();
    return 0;
}