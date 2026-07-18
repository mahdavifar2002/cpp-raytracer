cmake_minimum_required(VERSION 3.10)

# Define the name of your project
project(RayTracer VERSION 1.0)

# Set the C++ standard to C++17 (the ray tracing books use modern C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force aggressive optimization and architecture-specific instructions
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O3 -march=native -ffast-math)
elseif(MSVC)
    add_compile_options(/O2 /fp:fast)
endif()

# Define the executable name ("RayTracer") and its source and header files
add_executable(RayTracer src/main.cc)
target_include_directories(RayTracer PRIVATE include)

# Find and link OpenMP to multithread the render loop
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(RayTracer PUBLIC OpenMP::OpenMP_CXX)
endif()