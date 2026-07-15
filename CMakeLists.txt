cmake_minimum_required(VERSION 3.10)

# Define the name of your project
project(RayTracer VERSION 1.0)

# Set the C++ standard to C++17 (the ray tracing books use modern C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the executable name ("RayTracer") and its source files
add_executable(RayTracer main.cc)