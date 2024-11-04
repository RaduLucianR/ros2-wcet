import os
import time
import signal
import argparse
import subprocess

from stress import Stress
from introspection import introspect
from trigger_callbacks import trigger_subscriptions
from analyze_trace import compute_moet_from_trace

def main(session_name: str, 
         ros_pkg: str, launch_file: str, 
         publish_frequency: int, 
         nrof_messages: int
    ):

    stress = Stress()

    # Start tracing
    os.system(f"ros2 trace start {session_name}")

    # Start stressing the hardware platform on which we're running
    stress.stress_platform()

    # Start the whole ROS system
    ros_system = subprocess.Popen(['ros2', 'launch', ros_pkg, launch_file], start_new_session=True)
    time.sleep(5) # Wait for the system to be operational

    # Get the ROS system structure
    introspect()

    # Trigger the subscription callbacks
    trigger_subscriptions(publish_frequency, nrof_messages)
    time.sleep(1)

    # Stop and save collected traces
    os.system(f"ros2 trace stop {session_name}")

    # Clean-up: stop the stressing and terminate the ROS system
    stress.terminate()
    os.killpg(os.getpgid(ros_system.pid), signal.SIGTERM)

    # Compute the Maximum Observed Execution Times (MOET)
    compute_moet_from_trace(session_name)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("session_name", 
                        help = "Name of the folder that contains \
                                the Linux traces for this measuring session.",
                        type = str
                    )
    parser.add_argument("ros_pkg", 
                        help = "ROS package where the 'main' launch file \
                                is located, i.e. the file that starts all \
                                ROS nodes.",
                        type = str
                    )
    parser.add_argument("launch_file", 
                        help = "Name of the launch file that starts the entire \
                                ROS system, i.e. the file that starts all ROS \
                                nodes.",
                        type = str
                    )
    parser.add_argument("-f", "--publish_frequency",
                        help = "Frequency of publishing to a topic that triggers \
                                a callback.",
                        default = 100,
                        type = int
                        )
    parser.add_argument("-n", "--nrof_messages",
                        help = "Number of messages to be published to a topic that \
                                triggers a callback. One message corresponds to a trigger, \
                                and thus also to a timing measurement. So, the more messages \
                                the better is the MOET estimation.",
                        default = 10_000,
                        type = int
                        )
    args = parser.parse_args()

    main(args.session_name, 
         args.ros_pkg, 
         args.launch_file, 
         args.publish_frequency, 
         args.nrof_messages
        )