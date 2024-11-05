import os
import time
import signal
import argparse
import subprocess

from stress import Stress
from introspection import introspect
from trigger_callbacks import trigger_subscriptions
from analyze_trace import compute_moet_from_trace
from ros_parsers_bindings import *    

def measure_callbacks_moet(
         session_name: str, 
         ros_pkg: str, 
         launch_file: str, 
         publish_frequency: int, 
         nrof_messages: int,
         intrusive: bool,
         callback_type: str
    ):
    session_name = f"{session_name}_{callback_type}"
    stress = Stress()
    
    if intrusive == True:
        if callback_type == "subscription":
                apply_function_to_files_in_package(get_package_path(ros_pkg), disable_timers)
        elif callback_type == "timer":
                apply_function_to_files_in_package(get_package_path(ros_pkg), set_fast_periods)

        recompile_ros_ws(get_workspace_path(ros_pkg))

    # Start tracing
    os.system(f"ros2 trace start {session_name}")

    # Start stressing the hardware platform on which we're running
    stress.stress_platform()

    # Start the whole ROS system
    ros_system = subprocess.Popen(['ros2', 'launch', ros_pkg, launch_file], start_new_session=True)
    time.sleep(5) # Wait for the system to be operational

    # Trigger the subscription callbacks
    if callback_type == "subscription":
        introspect() # Introspection finds the topics that the trigger needs to publish to
        trigger_subscriptions(publish_frequency, nrof_messages)
        time.sleep(1)
    elif callback_type == "timer":
        # For timers there's no need for introspection since their callbacks are triggered
        # automatically by ROS. The tracing then finds the callbacks for us
        time.sleep(10)  # Since we set the timers to publish every 1ms
                        # then 10s should be enough to capture 10k samples
                        # but this doesn't take the scheduling interference
                        # so it might be that we have fewer than 10k samples.
                        # For now it should be ok.

    # Stop and save collected traces
    os.system(f"ros2 trace stop {session_name}")

    # Clean-up: stop the stressing and terminate the ROS system
    stress.terminate()
    os.killpg(os.getpgid(ros_system.pid), signal.SIGTERM)

    # Compute the Maximum Observed Execution Times (MOET)
    compute_moet_from_trace(session_name)

    # Revert source code modifications if there are any
    if intrusive == True:
        revert_modifications(get_package_path(ros_pkg))

def main(session_name: str, 
         ros_pkg: str, 
         launch_file: str, 
         publish_frequency: int, 
         nrof_messages: int,
         intrusive: bool
        ):
    if intrusive == True:
        get_pkg_compilation_database(ros_pkg)

    measure_callbacks_moet(session_name, ros_pkg, launch_file, 
                           publish_frequency, nrof_messages, intrusive,
                           "subscription")
    measure_callbacks_moet(session_name, ros_pkg, launch_file, 
                           publish_frequency, nrof_messages, intrusive,
                           "timer")

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
    parser.add_argument("-i", "--intrusive",
                        help = "The tool modifies the C++ source code of the timers to disable \
                                them or modify their period. This is done in order to prevent them \
                                from triggering subscription callbacks, so the tool can do that on \
                                its own, thus stressing the subscription callbacks independently of timers. \
                                Moreover, the timers' periods are modified to 1ms so we can record many  \
                                timers triggers faster, instead of waiting for each of their respective period. \
                                If you don't want to have intrusion then disable this, however the tool \
                                reverts the changes.",
                        default = True,
                        type = bool
                        )
    args = parser.parse_args()

    main(args.session_name, 
         args.ros_pkg, 
         args.launch_file, 
         args.publish_frequency, 
         args.nrof_messages,
         args.intrusive
        )