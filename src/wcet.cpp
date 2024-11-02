// Precondition: ROS system running. This has to be done by the user.
// i.e. the user starts up their entire ROS system which is comprised of
// all the ROS nodes. Ideally this would be done via a launch file or a script.

// Step 0: [OPTIONAL] pass a "main" launch file to WCET to start the ROS system automatically
// Step 1: introspect -> get a snapshot of the entire ROS system
// Step 2: shutdown ros2 system and then restart ros2 daemon
// Step 3: for each node
//          Step 3.1: Start ros2 trace with specific path
//          Step 3.2: Start stress program to stress the other cores
//          Step 3.3: Start node independently of the other nodes
//          Step 3.4: for each subscription of the node, publish 10k messages
//          Step 3.5: stop ros2 trace -> this saves the trace information
//          Step 3.6: run tracetools_analysis Python script to find MOET, add result to WCET file for the callback of the analyzed topic
#include <iostream>
#include <string>
#include <thread>

void start_node()
{
    system("ros2 run pub listener");
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "You did not provide a name for the tracing session! Format: wcet <tracing_session_name>";
        return EXIT_FAILURE;
    }

    //TODO: Check argv[1] is a string

    std::string traceSessionName = argv[1];
    std::string traceStartCmd = "ros2 trace start " + traceSessionName;
    std::string traceStopCmd = "ros2 trace stop " + traceSessionName;
    std::string traceAnalysisCmd = "python3 /home/radu/ros2-wcet/analyze_trace.py " + traceSessionName;

    system(traceStartCmd.c_str()); // Step 3.1
    //[start stress program]
    std::thread thread_obj(start_node); // Step 3.3 -> this way is not proper since we must join the threads.
                                        // Could be done with parametrized ros2 launch, but still we need threads.
    system("/home/radu/ros2-wcet/build/intro"); // Step 3.4 (ish). TODO: somehow make this path more general
    system(traceStopCmd.c_str());
    system(traceAnalysisCmd.c_str());
    //[cleanup -> kill nodes and ros2 and pyhton3]
}