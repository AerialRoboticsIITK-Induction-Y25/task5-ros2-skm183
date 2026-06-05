#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <vector>
#include <iomanip>
#include <algorithm>

class HealthMonitorNode : public rclcpp::Node {
public:
    HealthMonitorNode() : Node("health_monitor") {
        std::vector<std::string> drones = {"Alpha", "Beta", "Gamma"};

        warning_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_warning", 10);
        summary_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_summary", 10);

        for (const auto& d_name : drones) {
            std::string topic = "/drone/" + d_name + "/telemetry";
            auto sub = this->create_subscription<std_msgs::msg::String>(
                topic, 10,
                [this, d_name](const std_msgs::msg::String::SharedPtr msg) {
                    this->process_telemetry(d_name, msg->data);
                });
            telemetry_subs_.push_back(sub);
            
            current_battery_[d_name] = 100.0;
            drain_rates_[d_name] = 0.0;
        }

        diag_timer_ = this->create_wall_timer(
            std::chrono::seconds(10), std::bind(&HealthMonitorNode::publish_diagnostics, this));

        RCLCPP_INFO(this->get_logger(), "Health Monitor Node Initialized.");
    }

private:
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_;
    
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr warning_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr summary_pub_;
    rclcpp::TimerBase::SharedPtr diag_timer_;

    std::map<std::string, std::deque<std::pair<double, double>>> history_buffer_;
    std::map<std::string, double> current_battery_;
    std::map<std::string, double> drain_rates_;

    double extract_battery(std::string json_str) {
        json_str.erase(std::remove(json_str.begin(), json_str.end(), '{'), json_str.end());
        json_str.erase(std::remove(json_str.begin(), json_str.end(), '}'), json_str.end());
        json_str.erase(std::remove(json_str.begin(), json_str.end(), '"'), json_str.end());
        json_str.erase(std::remove(json_str.begin(), json_str.end(), ' '), json_str.end());

        std::istringstream ss(json_str);
        std::string pair;
        while (std::getline(ss, pair, ',')) {
            size_t delim = pair.find(':');
            if (delim != std::string::npos) {
                if (pair.substr(0, delim) == "battery") {
                    try { return std::stod(pair.substr(delim + 1)); } 
                    catch (...) { return -1.0; }
                }
            }
        }
        return -1.0;
    }

    void process_telemetry(const std::string& drone_name, const std::string& data) {
        double battery = extract_battery(data);
        if (battery < 0.0) return;

        current_battery_[drone_name] = battery;
        double current_time = this->now().seconds();

        auto& buffer = history_buffer_[drone_name];
        buffer.push_back({current_time, battery});

        if (buffer.size() > 10) {
            buffer.pop_front();
        }

        if (buffer.size() >= 2) {
            double dt = buffer.back().first - buffer.front().first;
            double db = buffer.front().second - buffer.back().second;
            if (dt > 0) {
                double rate = db / dt;
                drain_rates_[drone_name] = rate;

                if (rate > 1.5) {
                    auto warn_msg = std_msgs::msg::String();
                    warn_msg.data = "[WARNING] " + drone_name + " high drain rate: " + std::to_string(rate) + " %/s";
                    warning_pub_->publish(warn_msg);
                    RCLCPP_WARN(this->get_logger(), "%s", warn_msg.data.c_str());
                }
            }
        }
    }

    void publish_diagnostics() {
        std::cout << "\n[HEALTH DIAGNOSTICS - 10s REPORT]\n";
        std::cout << std::left << std::setw(10) << "DRONE" 
                  << std::setw(15) << "DRAIN (%/s)" 
                  << std::setw(20) << "TIME TO CRIT (20%)" 
                  << std::setw(20) << "TIME TO DEPLETION" << "\n";
        
        std::vector<std::string> order = {"Alpha", "Beta", "Gamma"};
        std::string json_summary = "{";
        
        for (size_t i = 0; i < order.size(); ++i) {
            std::string name = order[i];
            double rate = drain_rates_[name];
            double batt = current_battery_[name];

            std::string time_to_crit = "N/A";
            std::string time_to_empty = "N/A";

            if (rate > 0) {
                double t_crit = (batt - 20.0) / rate;
                double t_empty = batt / rate;
                
                time_to_crit = (t_crit > 0) ? std::to_string((int)t_crit) + "s" : "CRITICAL";
                time_to_empty = std::to_string((int)t_empty) + "s";
            }

            std::cout << std::left << std::setw(10) << name 
                      << std::setw(15) << std::fixed << std::setprecision(2) << rate 
                      << std::setw(20) << time_to_crit 
                      << std::setw(20) << time_to_empty << "\n";

            json_summary += "\"" + name + "\":{\"rate\":" + std::to_string(rate) + "}";
            if (i < order.size() - 1) json_summary += ",";
        }
        
        json_summary += "}";

        auto msg = std_msgs::msg::String();
        msg.data = json_summary;
        summary_pub_->publish(msg);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<HealthMonitorNode>());
    rclcpp::shutdown();
    return 0;
}
