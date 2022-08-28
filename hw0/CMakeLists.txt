cmake_minimum_required(VERSION 3.10)

# set the project name
project(homework0)

set(CMAKE_COLOR_MAKEFILE ON)

set_target_properties(PROPERTIES OUTPUT_NAME homework0)

# set(EXECUTABLE_OUTPUT_PATH build)

# add the executable
add_executable(hw0 src/hw0.cpp)
