cmake_minimum_required(VERSION 3.10)

# set(CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew/opt/nlohmann-json/share/cmake/nlohmann_json")

project(RosParsers)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
include_directories(include)

add_executable(ros_disable_timers src/ros_disable_timers.cpp)
target_link_libraries(ros_disable_timers # Link against LLVM and Clang libraries
    ${LLVM_LIBRARIES}
    ${CLANG_LIBRARIES}
    clangTooling
    clangBasic
    clangAST
    clangSupport
)

# Specify C++ standard
set_target_properties(ros_disable_timers PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Optionally, if you want to include Clang tooling support
target_compile_definitions(ros_disable_timers PRIVATE
    -DCLANG_TOOLING_BUILD
)

add_executable(ros_fast_period_timers src/ros_fast_period_timers.cpp)
target_link_libraries(ros_fast_period_timers # Link against LLVM and Clang libraries
    ${LLVM_LIBRARIES}
    ${CLANG_LIBRARIES}
    clangTooling
    clangBasic
    clangAST
    clangSupport
)

add_executable(ros_callback_extractor src/ros_callback_extractor.cpp)
target_link_libraries(ros_callback_extractor # Link against LLVM and Clang libraries
    ${LLVM_LIBRARIES}
    ${CLANG_LIBRARIES}
    clangTooling
    clangBasic
    clangAST
    clangSupport
    nlohmann_json # JSON lib
)

# Specify C++ standard
set_target_properties(ros_fast_period_timers PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Optionally, if you want to include Clang tooling support
target_compile_definitions(ros_fast_period_timers PRIVATE
    -DCLANG_TOOLING_BUILD
)
