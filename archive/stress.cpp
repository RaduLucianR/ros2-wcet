#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstring>

// Size of memory block to access (in bytes)
const size_t MEMORY_BLOCK_SIZE = 256 * 1024 * 1024; // 256 MB

// Function that performs a combination of CPU and memory stress
void stress_cpu_memory(int core_id) {
    // Bind the thread to a specific core (optional, depends on system)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    std::cout << "Starting CPU and memory stress on core " << core_id << std::endl;

    // Allocate a large block of memory to cause cache/memory pressure
    char* memory_block = new char[MEMORY_BLOCK_SIZE];
    
    // Fill the memory with some data to simulate memory accesses
    memset(memory_block, 1, MEMORY_BLOCK_SIZE);

    volatile double result = 0.0;

    while (true) {
        // Memory-intensive part: walk through the memory block
        for (size_t i = 0; i < MEMORY_BLOCK_SIZE; i += 64) {
            result += std::sin(i) * std::cos(i);
            memory_block[i] += 1; // Access memory every 64 bytes (cache line size)
        }

        // Small sleep to avoid complete system overloading
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    // Clean up
    delete[] memory_block;
}

int main() {
    // Get the number of available CPU cores
    unsigned int num_cores = std::thread::hardware_concurrency();
    std::cout << "System has " << num_cores << " cores available.\n";

    // Create a vector to hold our threads
    std::vector<std::thread> threads;

    // Launch threads to stress all cores except core 0 (to leave one core for ROS2)
    for (unsigned int i = 1; i < num_cores; ++i) {
        threads.push_back(std::thread(stress_cpu_memory, i));
    }

    // Optionally, join threads (or detach them if you don't want to join)
    for (auto& t : threads) {
        t.detach();  // Detach so they run independently
    }

    // Allow stress program to run indefinitely
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}