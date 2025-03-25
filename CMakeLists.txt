cmake_minimum_required(VERSION 3.26)
project(pm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(pm tests/dialog_resizing_graphical_object.cpp
        src/constraint.cpp
        src/priority.cpp
        kernel/delta_blue.cpp
        src/variable.cpp
        src/constraint_graph.cpp)
