cmake_minimum_required(VERSION 3.0.0)
project(nodepp-gpu VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(NODEPP_GPU_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Search for Nodepp Core
if (NOT TARGET nodepp)
    find_package(nodepp REQUIRED)
    if(NOT nodepp_FOUND)
        message(FATAL_ERROR "Nodepp not found. Please install nodepp.")
    endif()
endif()

# Search for Nodepp-raylib
if (NOT TARGET nodepp-raylib)
    find_package(nodepp-raylib REQUIRED)
    if(NOT nodepp_FOUND)
        message(FATAL_ERROR "Nodepp not found. Please install nodepp-raylib.")
    endif()
endif()

# interface-target
add_library(nodepp-gpu INTERFACE)
target_include_directories(nodepp-gpu INTERFACE ${NODEPP_GPU_INCLUDE_DIR})
target_link_libraries(nodepp-gpu INTERFACE nodepp nodepp-raylib ) #------#