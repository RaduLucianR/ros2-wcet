import os
import subprocess
import json

def introspect():
    # Constants and variables
    user = os.getenv("USER")
    path_to_main_dir = f"/home/{user}/.ros2wcet/"
    node_list_file = "nodeList.txt"
    node_info_file = "nodeInfoTmp.txt"
    node_list_cmd = "ros2 node list"
    node_info_cmd = "ros2 node info"
    json_file = "nodes.json"
    nodes = []  # List of node names
    subscribers = {}
    publishers = {}

    # Get list of nodes from ROS2 into the list of nodes
    command = f"{node_list_cmd} > {path_to_main_dir}{node_list_file}"
    subprocess.run(command, shell=True)

    # Read nodes from file
    with open(path_to_main_dir + node_list_file, 'r') as file_stream:
        nodes = [line.strip() for line in file_stream]

    # For each node, get its subscribers and publishers
    for node in nodes:
        node_sub = []
        node_pub = []
        command = f"{node_info_cmd} {node} > {path_to_main_dir}{node_info_file}"
        subprocess.run(command, shell=True)

        sub_line_found = False
        pub_line_found = False

        with open(path_to_main_dir + node_info_file, 'r') as file_stream:
            for line in file_stream:
                line = line.strip()
                if "Subscribers:" in line:
                    sub_line_found = True
                    pub_line_found = False
                    continue
                elif "Publishers:" in line:
                    pub_line_found = True
                    sub_line_found = False
                    continue
                elif "Service Servers:" in line:
                    sub_line_found = False
                    pub_line_found = False
                    break

                if sub_line_found or pub_line_found:
                    slash_pos = line.find("/")
                    colon_pos = line.find(":")
                    topic = line[slash_pos:colon_pos].strip()
                    type_ = line[colon_pos + 2:].strip()
                    topic_and_type = (topic, type_)
                    
                    if sub_line_found:
                        node_sub.append(topic_and_type)
                    elif pub_line_found:
                        node_pub.append(topic_and_type)

        subscribers[node] = node_sub
        publishers[node] = node_pub

    # Convert map to JSON
    nodes_json = []
    for node in nodes:
        node_json = {
            "node": node,
            "subscribers": [{"topic": t, "type": tp} for t, tp in subscribers.get(node, [])],
            "publishers": [{"topic": t, "type": tp} for t, tp in publishers.get(node, [])]
        }
        nodes_json.append(node_json)

    # Write to JSON file
    with open(path_to_main_dir + json_file, 'w') as file_stream:
        json.dump(nodes_json, file_stream, indent=4)
