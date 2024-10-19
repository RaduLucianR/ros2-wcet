cmake_minimum_required(VERSION 3.10)

set(CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew/opt/nlohmann-json/share/cmake/nlohmann_json")

project(MyClangTool)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Find Clang package
find_package(Clang REQUIRED CONFIG)


find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

# Add executable target
add_executable(rosParser src/ros_parser.cpp)

# Link against LLVM and Clang libraries
target_link_libraries(rosParser
    ${LLVM_LIBRARIES}
    ${CLANG_LIBRARIES}
    clangTooling
    clangBasic
    clangAST
    clangSupport
    nlohmann_json::nlohmann_json
)


# Specify C++ standard
set_target_properties(rosParser PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Optionally, if you want to include Clang tooling support
target_compile_definitions(rosParser PRIVATE
    -DCLANG_TOOLING_BUILD
)