#!/bin/bash

# if [ -z $1 ]; then
#     echo "You must provide a session name!"
#     exit 1
# fi

# SESH_NAME=$1
# ros2 trace start $SESH_NAME
# /home/radu/ros2-wcet/build/stress &
# PID_STRESS=$!
ros2 launch pub main_launch.py &
PID_ROSSYSTEM=$!
sleep 5
# /home/radu/ros2-wcet/build/intro
# sleep 1
# ros2 trace stop $SESH_NAME
# python3 /home/radu/ros2-wcet/analyze_trace.py $SESH_NAME
# kill -9 $PID_STRESS
pkill -P $PID_ROSSYSTEM
