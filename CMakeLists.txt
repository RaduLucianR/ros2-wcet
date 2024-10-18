cmake_minimum_required(VERSION 3.10)
project(MyClangTool)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Find Clang package
find_package(Clang REQUIRED CONFIG)

# Include directories
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

# Add executable target
add_executable(my_clang_tool src/lib_tooling_example.cpp)
add_executable(test src/test.cpp)

# Link against LLVM and Clang libraries
target_link_libraries(my_clang_tool
    ${LLVM_LIBRARIES}
    ${CLANG_LIBRARIES}
    clangTooling
    clangBasic
    clangAST
    clangSupport
)


# Specify C++ standard
set_target_properties(my_clang_tool PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Optionally, if you want to include Clang tooling support
target_compile_definitions(my_clang_tool PRIVATE
    -DCLANG_TOOLING_BUILD
)