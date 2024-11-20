import subprocess
import os
import fnmatch
import shutil

# TODO: Make actual bindings with pybind

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

def get_pkg_compilation_database(package_name: str):
    pkg_path = get_package_path(package_name)
    default_ros2wcet_folder = os.path.expanduser("~/.ros2wcet/")
    cmake_output_folder = os.path.join(default_ros2wcet_folder, "cmake_garbage")
    os.makedirs(cmake_output_folder, exist_ok=True)
    os.chdir(os.path.expanduser(cmake_output_folder))
    subprocess.run(["cmake", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", pkg_path], check = True)
    compilation_database_path = os.path.join(cmake_output_folder, "compile_commands.json")
    destination_path = os.path.join(pkg_path, "compile_commands.json")
    shutil.copy2(compilation_database_path, destination_path)
    # shutil.rmtree(cmake_output_folder)

def remove_cmake_garbage(package_name: str):
    pkg_path = get_package_path(package_name)
    destination_path = os.path.join(pkg_path, "compile_commands.json")
    os.remove(destination_path)
    default_ros2wcet_folder = os.path.expanduser("~/.ros2wcet/")
    cmake_output_folder = os.path.join(default_ros2wcet_folder, "cmake_garbage")
    shutil.rmtree(cmake_output_folder)


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

def revert_modifications(src_folder, backup_folder="~/.ros2wcet/save_modified_files"):
    backup_folder = os.path.expanduser(backup_folder)

    for file_name in os.listdir(backup_folder):
        # Full path of the file in the backup folder
        backup_file_path = os.path.join(backup_folder, file_name)
        
        # Find the target file within the src_folder
        for root, _, files in os.walk(src_folder):
            if file_name in files:
                target_file_path = os.path.join(root, file_name)
                
                # Copy the backup file to the target location, overwriting the modified file
                shutil.copy2(backup_file_path, target_file_path)
                print(f"Reverted: {target_file_path}")
                break
        else:
            print(f"Warning: {file_name} not found in {src_folder}")
    
    shutil.rmtree(backup_folder)