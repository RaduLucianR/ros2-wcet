import multiprocessing
import numpy as np
import os

class Stress:
    def __init__(self):
        self.processes = []

    def stress_core(self, core_id):
        # Bind process to a specific CPU core
        os.sched_setaffinity(0, {core_id})
        
        # Create a large array to stress memory and maximize cache misses
        size = 10000000  # Adjust as needed to increase memory usage
        array = np.random.rand(size)
        
        # Access the array randomly to increase cache misses
        while True:
            # Generate random indices and access array elements
            indices = np.random.randint(0, size, size // 10)  # Random access pattern
            for i in indices:
                array[i] *= np.pi  # Arbitrary computation to keep CPU busy

    def stress_platform(self):
        # Get the number of available cores
        num_cores = os.cpu_count()
        
        for core_id in range(1, num_cores):
            process = multiprocessing.Process(target = self.stress_core, args=(core_id,))
            process.start()
            self.processes.append(process)
        
    def terminate(self):
        for process in self.processes:
            process.terminate()

        for process in self.processes:
            process.join()
