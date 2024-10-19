#include "measure.hpp"
#include <memory>
#include "pub.hpp"

int main(int argc, char* argv[]) {
    std::string csv_location = "data.csv";
    rclcpp::init(argc, argv);
    std::shared_ptr obj = std::make_shared<MinimalPublisher>();
    measure(&MinimalPublisher::timer_callback, obj, csv_location);
    rclcpp::shutdown();
}