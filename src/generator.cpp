#include <fstream>
#include <nlohmann/json.hpp>

int main(int argc, char* argv[])
{
    std::ifstream JsonFile("pub.hpp.json");
    nlohmann::json Nodes;
    JsonFile >> Nodes;
    JsonFile.close();

    std::string node = Nodes[0]["name"];
    std::string callback = Nodes[0]["callbacks"][0]["name"];

    std::string includeNodeHeader = "#include \"pub.hpp\"\n\n";
    std::string initNode = "    std::shared_ptr obj = std::make_shared<" + node + ">();\n";
    std::string measurement = "    measure(&" + node + "::" + callback + ", obj, csv_location);\n";

    std::string testbenchFile =   
                "#include \"measure.hpp\"\n"
                "#include <memory>\n"
                + includeNodeHeader +
                "int main(int argc, char* argv[]) {\n"
                "    std::string csv_location = \"data.csv\";\n"
                "    rclcpp::init(argc, argv);\n"
                + initNode +
                measurement +
                "    rclcpp::shutdown();\n"
                "}\n";
    
    std::ofstream TBcpp("testbenchTest.cpp");
    TBcpp << testbenchFile;
    TBcpp.close();
}