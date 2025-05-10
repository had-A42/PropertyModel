cmake_minimum_required(VERSION 3.26)
project(pm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(pm tests/dialog_resizing.cpp
        src/constraint.cpp
        src/priority.cpp
        src/variable.cpp
        src/constraint_graph.cpp
        src/except.cpp
        kernel/delta_blue.cpp)

target_include_directories(pm PUBLIC ${PROJECT_SOURCE_DIR}/include)
