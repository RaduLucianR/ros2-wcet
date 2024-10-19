/**
 * ###### Generated file ######
 */

#include "measure.hpp"
#include <memory>
// #include "rclcpp/rclcpp.hpp"
// #include [file_with_node]

class Hey {
public:
    int sum() {
        int s = 0;

        for (int i = 0; i < 10000; i ++)
        {
            s += 1;
        }
        
        return s;
    }

    int mul(int a) {
        int s = 0;

        for (int i = 0; i < 10000; i ++)
        {
            s += 1;
        }
        
        return s;
    }
};

int main() {
    std::string csv_location = "data.csv";
    // node = [instantiate node]
    std::shared_ptr obj = std::make_shared<Hey>();

    // arguments = [generate arguments]

    // measure([&NodeClass::callback], obj, csv_location, [arguments]);
    measure(&Hey::sum, obj, csv_location);
    measure(&Hey::mul, obj, csv_location, 10);

    return 0;
}