import subprocess
import os
import fnmatch

def disable_timers(file: str):
    subprocess.run(["/home/radu/ros2-wcet/build/ros_disable_timers", file])

def set_fast_periods(file: str):
    subprocess.run(["/home/radu/ros2-wcet/build/ros_fast_period_timers", file])

def recompile_ros_ws(workspace_path: str):
    os.chdir(workspace_path)
    subprocess.run(["colcon", "build"], check = True)

def get_package_path(package_name: str):
    ws_path = get_workspace_path(package_name)

    return ws_path + "/src/" + package_name

def get_workspace_path(package_name: str):
    cli_result = subprocess.run(["ros2", "pkg", "prefix", package_name], 
                          stdout=subprocess.PIPE, 
                          check = True
    )

    path = cli_result.stdout
    install_dir = os.path.dirname(path)
    workspace_dir = os.path.dirname(install_dir)

    return workspace_dir.decode() # Convert from byte string to string

def apply_function_to_files_in_package(base_dir, func, target_subfolders = ['src', 'include']):
    for root, dirs, files in os.walk(base_dir):
        # Check if we are in a target directory
        if any(fnmatch.fnmatch(os.path.basename(root), d) for d in target_subfolders):
            for filename in files:
                if fnmatch.fnmatch(filename, '*.cpp') or fnmatch.fnmatch(filename, '*.hpp'):
                    file_path = os.path.join(root, filename)
                    func(file_path)