cmake_minimum_required(VERSION 3.10)

project(VTK_Diff)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(VTK_Diff main.cpp)
target_link_libraries(VTK_Diff ${VTK_LIBRARIES})
