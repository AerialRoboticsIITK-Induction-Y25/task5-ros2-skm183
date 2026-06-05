#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include "mission_drone.hpp"
#include "drone_exceptions.hpp"
#include <sstream>

class DroneNode : public rclcpp::Node {
public:
    DroneNode() : Node("drone_node") {
        this->declare_parameter<std::string>("drone_name", "Unknown");
        this->declare_parameter<double>("initial_battery", 100.0);
        this->declare_parameter<std::string>("mission_name", "Default_Mission");

        std::string d_name = this->get_parameter("drone_name").as_string();
        double i_battery = this->get_parameter("initial_battery").as_double();
        
        drone_.name = d_name;
        
        if (i_battery < 100.0) {
            drone_.drain_battery(100.0 - i_battery);
        }
        
        drone_.set_status("flying");

        drone_.waypoints = {
            {10.0, 0.0, 5.0}, {20.0, 10.0, 5.0}, {30.0, 0.0, 5.0}, 
            {40.0, -10.0, 5.0}, {50.0, 0.0, 5.0}
        };

        std::string base_topic = "/drone/" + d_name;
        status_pub_ = this->create_publisher<std_msgs::msg::String>(base_topic + "/status", 10);
        alert_pub_ = this->create_publisher<std_msgs::msg::String>(base_topic + "/alert", 10);
        mission_pub_ = this->create_publisher<std_msgs::msg::String>(base_topic + "/mission_complete", 10);
        telemetry_pub_ = this->create_publisher<std_msgs::msg::String>(base_topic + "/telemetry", 10);

        status_timer_ = this->create_wall_timer(
            std::chrono::seconds(1), std::bind(&DroneNode::publish_status, this));
        
        telemetry_timer_ = this->create_wall_timer(
            std::chrono::seconds(2), std::bind(&DroneNode::publish_telemetry, this));

        RCLCPP_INFO(this->get_logger(), "Drone Node '%s' initialized.", d_name.c_str());
    }

private:
    MissionDrone drone_;
    int publish_count_ = 0;

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr alert_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mission_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr telemetry_pub_;
    
    rclcpp::TimerBase::SharedPtr status_timer_;
    rclcpp::TimerBase::SharedPtr telemetry_timer_;

    void publish_status() {
        if (drone_.get_status() != "flying") return;

        try {
            drone_.drain_battery(0.5f);
        } catch (const BatteryDepletedError& e) {
            auto alert_msg = std_msgs::msg::String();
            alert_msg.data = "CRITICAL BATTERY";
            alert_pub_->publish(alert_msg);
            drone_.set_status("idle");
            return;
        }

        publish_count_++;
        if (publish_count_ % 3 == 0) {
            if (!drone_.mission_complete()) {
                drone_.next_waypoint();
            } else {
                auto m_msg = std_msgs::msg::String();
                m_msg.data = "MISSION COMPLETE";
                mission_pub_->publish(m_msg);
                
		            drone_.current_waypoint_index = 0;
            }
        }

        auto msg = std_msgs::msg::String();
        std::stringstream ss;
        ss << "name:" << drone_.name 
           << "|battery:" << drone_.get_battery_level() 
           << "|altitude:5.0" 
           << "|status:" << drone_.get_status()
           << "|waypoint:" << publish_count_/3 << "/5"
           << "|speed:3.2";
        
        msg.data = ss.str();
        status_pub_->publish(msg);
    }

    void publish_telemetry() {
        auto msg = std_msgs::msg::String();
        std::stringstream ss;
        ss << "{\"name\":\"" << drone_.name << "\", "
           << "\"battery\":" << drone_.get_battery_level() << ", "
           << "\"status\":\"" << drone_.get_status() << "\"}"; 
        
        msg.data = ss.str();
        telemetry_pub_->publish(msg);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DroneNode>());
    rclcpp::shutdown();
    return 0;
}
