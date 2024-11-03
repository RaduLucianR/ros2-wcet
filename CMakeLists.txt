cmake_minimum_required(VERSION 3.10)

set(CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew/opt/nlohmann-json/share/cmake/nlohmann_json")

project(MyClangTool)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
include_directories(include)

# Add executable target
add_executable(rosParser src/ros_parser.cpp)
target_link_libraries(rosParser # Link against LLVM and Clang libraries
    ${LLVM_LIBRARIES}
    ${CLANG_LIBRARIES}
    clangTooling
    clangBasic
    clangAST
    clangSupport
    nlohmann_json::nlohmann_json
)

# add_executable(testbench src/testbench.cpp)

add_executable(generator src/generator.cpp)
target_link_libraries(generator
    nlohmann_json::nlohmann_json
)

add_executable(intro src/introspect.cpp)

add_executable(wcet src/wcet.cpp)

add_executable(stress src/stress.cpp)


# Specify C++ standard
set_target_properties(rosParser PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Optionally, if you want to include Clang tooling support
target_compile_definitions(rosParser PRIVATE
    -DCLANG_TOOLING_BUILD
)