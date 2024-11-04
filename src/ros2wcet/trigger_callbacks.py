import subprocess
import json
import os

def trigger_subscriptions(publish_frequency = 100, nrof_messages = 10_000):
    user = os.getenv("USER")
    path_to_json = f"/home/{user}/.ros2wcet/nodes.json"
    node_pub_cmd = f"ros2 topic pub -r {publish_frequency} -t {nrof_messages}"
    nodes = []  # List of node names
    subscribers = {}

    with open(path_to_json, 'r') as f:
        nodes = json.load(f)


    # Publish many messages
    for node in nodes:
        subscribers = node.get("subscribers", [])
        
        for subscriber in subscribers:
            topic = subscriber["topic"]
            msg_type = subscriber["type"]

            if topic != "/parameter_events":  # Filter out default subscription
                command = f"{node_pub_cmd} {topic} {msg_type}"
                subprocess.run(command, shell = True)