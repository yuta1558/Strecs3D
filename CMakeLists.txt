cmake_minimum_required(VERSION 3.10)

project(VTK_Diff)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# VTK & CGAL を見つけて使用
find_package(VTK REQUIRED)
find_package(CGAL REQUIRED COMPONENTS Core)

if (NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found")
endif()

include(${VTK_USE_FILE})

# lib3mf のパス設定
set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin")
find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIB3MF_LIB)
    message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
else()
    message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
endif()

add_executable(VTK_Diff
    main.cpp
    VtkProcessor.cpp
    cgalProcessor.cpp
)

# インクルードパス
target_include_directories(VTK_Diff PRIVATE 
    "${LIB3MF_ROOT}/include"
    "${LIB3MF_ROOT}/include/Bindings/Cpp"
)

# リンクするライブラリ
target_link_libraries(VTK_Diff
  PRIVATE
    ${VTK_LIBRARIES}
    CGAL::CGAL
    "${LIB3MF_LIB}"
)
