import sys

# TODO: Understand how this import works. It should throw an error!?
sys.path.insert(0, '../')
sys.path.insert(0, '../../../ros2/ros2_tracing/tracetools_read/')

import datetime as dt
import numpy as np
import pandas as pd
import argparse
import os
import json

from tracetools_analysis.loading import load_file
from tracetools_analysis.processor.ros2 import Ros2Handler
from tracetools_analysis.utils.ros2 import Ros2DataModelUtil

def compute_moet_from_trace(trace_session, toplevel_trace_dir = "~/.ros/tracing"):
    json_path = os.path.expanduser("~/.ros2wcet/moet.json")

    with open(json_path, 'r') as file:
        moet = json.load(file) # Maximum Observed Execution Times

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
        moet_callback = duration_sec.max()

        # If the entry exists then update only if 
        # the new value is larger, i.e. only keep the maximum
        if obj in moet:
            if moet_callback > moet[obj]:
                moet[obj] = moet_callback
        else:
            moet[obj] = moet_callback
        
        print(obj, "has MOET:", moet[obj], "ms")

    with open(json_path, "w") as outfile: 
        json.dump(moet, outfile, indent = 4)