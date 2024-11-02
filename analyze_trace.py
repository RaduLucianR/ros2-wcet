import sys
# Add paths to tracetools_analysis and tracetools_read.
# There are two options:
#   1. from source, assuming a workspace with:
#       src/tracetools_analysis/
#       src/ros2/ros2_tracing/tracetools_read/
sys.path.insert(0, '../')
sys.path.insert(0, '../../../ros2/ros2_tracing/tracetools_read/') # TODO: Understand how this import works. It should throw an error!?
#   2. from Debian packages, setting the right ROS 2 distro:
#ROS_DISTRO = 'rolling'
#sys.path.insert(0, f'/opt/ros/{ROS_DISTRO}/lib/python3.8/site-packages')
import datetime as dt
import numpy as np
import pandas as pd
import argparse
import os


from tracetools_analysis.loading import load_file
from tracetools_analysis.processor.ros2 import Ros2Handler
from tracetools_analysis.utils.ros2 import Ros2DataModelUtil

def main(trace_session, toplevel_trace_dir = "~/.ros/tracing"):
    path = os.path.join(toplevel_trace_dir, trace_session, "ust")
    events = load_file(path)
    handler = Ros2Handler.process(events)
    data_util = Ros2DataModelUtil(handler.data)
    callback_symbols = data_util.get_callback_symbols()

    for obj, symbol in callback_symbols.items():
        owner_info = data_util.get_callback_owner_info(obj)

        if owner_info is None:
            owner_info = '[unknown]'

        # Filter out internal subscriptions
        if '/parameter_events' in owner_info:
            continue

        # Duration
        duration_df = data_util.get_callback_durations(obj)
        duration_sec = duration_df['duration'] * 1000 / np.timedelta64(1, 's')

        print(symbol, "has MOET: ", duration_sec.max(), "ms")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("trace_folder", 
                        help="Name of folder with traces. This assumes that ~/.ros/tracing is the default directory where folder with traces are stored.",
                        type=str
                    )
    args = parser.parse_args()

    main(args.trace_folder)