#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <map>

#include "nlohmann/json.hpp"

using namespace nlohmann;

int main(int argc, char* argv[])
{
    // Constants and variables
    const char* user = std::getenv("USER");
    std::string userStr(user);
    std::string pathToMainDir = "/home/" + userStr + "/.ros2wcet/";
    std::string nodeListFile = "nodeList.txt";
    std::string nodeInfoFile = "nodeInfoTmp.txt";
    std::string nodeListCmd = "ros2 node list";
    std::string nodeInfoCmd = "ros2 node info";
    std::string nodePubCmd = "ros2 topic pub -r 10 -t 10";
    std::string jsonFile = "nodes.json";
    std::fstream fileStream;    // Fstream to open temporary files
    std::string line = "";      // String to temporary store a line from a file
    std::vector<std::string> nodes; // List of names of nodes
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> subscribers;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> publishers;

    // Get list of nodes from ROS2 into the vector of nodes
    std::string command = nodeListCmd + " > " + pathToMainDir + nodeListFile;
    system(command.c_str());

    fileStream.open(pathToMainDir + nodeListFile, std::ios::in);

    if (fileStream.is_open()) {
        while(std::getline(fileStream, line)) {
            nodes.push_back(line);
        }

        fileStream.close();
    }

    // For each node get its subscribers and publishers
    bool subLineFound = false;
    bool pubLineFound = false;

    for (std::string node : nodes) {
        std::vector<std::pair<std::string, std::string>> nodeSub;
        std::vector<std::pair<std::string, std::string>> nodePub;

        command = nodeInfoCmd + " " + node + " > " + pathToMainDir + nodeInfoFile;
        system(command.c_str());
        fileStream.open(pathToMainDir + nodeInfoFile, std::ios::in);

        if (fileStream.is_open()) {
            while(std::getline(fileStream, line)) {
                // string::find returns string::npos if substring not found
                if (line.find("Subscribers:") != std::string::npos) {
                    subLineFound = true;
                    continue;
                }

                if (line.find("Publishers:") != std::string::npos) {
                    pubLineFound = true;
                    subLineFound = false;
                    continue;
                }

                if (line.find("Service Servers:") != std::string::npos) {
                    subLineFound = false;
                    pubLineFound = false;
                    break;
                }

                if (subLineFound == true || pubLineFound == true) {
                    int slashPos = line.find("/");
                    int colonPos = line.find(":");
                    std::string topic = line.substr(slashPos, colonPos - slashPos);
                    std::string type = line.substr(colonPos + 2);
                    std::pair<std::string, std::string> topicAndType(topic, type);
                    
                    if (subLineFound == true && pubLineFound == false) {
                        nodeSub.push_back(topicAndType);
                    } else if (subLineFound == false && pubLineFound == true) {
                        nodePub.push_back(topicAndType);
                    }
                }
            }

            fileStream.close();
        }

        subscribers[node] = nodeSub;
        publishers[node] = nodePub;
    }

    // Publish 10000 messages
    for (std::string node: nodes) {
        for (std::pair sub : subscribers[node]) {
            std::string topic = sub.first;
            std::string msg_type = sub.second;
            command = nodePubCmd + " " + topic + " " + msg_type;
            system(command.c_str());
        }
    }

    // Convert map to json
    int nodeCounter = 0;
    json nodesJson = json::array();

    for (std::string node : nodes) {
        int subCounter = 0;
        int pubCounter = 0;
        json nodeJson;

        nodeJson["node"] = node;
        nodeJson["subscribers"] = json::array();

        for (std::pair tt : subscribers[node]) {
            json topicJson;

            topicJson["topic"] = tt.first;
            topicJson["type"] = tt.second; 
            nodeJson["subscribers"][subCounter] = topicJson;
            subCounter ++;
        }

        for (std::pair tt : publishers[node]) {
            json topicJson;

            topicJson["topic"] = tt.first;
            topicJson["type"] = tt.second; 
            nodeJson["publishers"][pubCounter] = topicJson;
            pubCounter ++;
        }

        nodesJson[nodeCounter] = nodeJson;
        nodeCounter ++;
    }

    // Write to JSON file
    fileStream.open(pathToMainDir + jsonFile, std::ios::out);
    fileStream << nodesJson.dump(4) << std::endl;
    fileStream.close();
}