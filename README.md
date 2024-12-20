# ros2-wcet
Automatically measure the WCET of callbacks in ROS2. THIS IS STILL WIP (WORK-IN-PROGRESS).

# Software design
This section details the design decisions taken to create this software tool.

## Assumptions
As this tool cannot cover all possible cases, we assume the following about the ROS2 code base that the tool is supposed to analyze.

1. All callbacks are public member methods of their Node.
    - Explanation: The tool needs to call a callback to perform a measurement. It needs to do this many times (i.e. in the order of 10^4). The overhead of ROS is too great to wait for a timer callback T * 10000 (where T is its period). Further, subscribtion callbacks cannot be measured including their function call unless the internal ROS2 code is modified, because the ROS2 executor is the one who calls the callback. Thus, this tool needs to instantiate a Node class into an object, and then call the callbacks with that object. Therefore, callbacks cannot be private functions or lambda functions.
2. Timer callbacks do not have parameters.
3. Subscription callbacks only process messages that are available in vanilla ROS2 Jazzy.
4. Declaration of Nodes is done in header (`.hpp`) files.
5. The cpp files implementing Nodes do not contain `main()` functions.
6. The cpp files implementing Nodes must be compiled as libraries by cmake (i.e. with `add_library`). This is because this tool cannot find the dependencies for the Node files by itself, so they must be provided.
7. The libraries must have the same name as their `.hpp` and `.cpp` files. So a node "per" will have files `per.hpp` and `per.cpp` and will be compiled as a library as `add_library(per [dependencies])`

## System specification
This tool was developed for ROS2 Jazzy on the Pro version of Ubuntu 24.02 with the PREEMPT_RT patch applied to the Linux kernel. The kernel version is `6.8.1-1009-realtime`.
