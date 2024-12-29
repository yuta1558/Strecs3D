cmake_minimum_required(VERSION 3.10)

project(VTK_Diff)

# VTKを見つけて使用する
find_package(VTK REQUIRED)
find_package(CGAL REQUIRED COMPONENTS Core)
if (NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found")
endif()

# VTKの設定を含める
include(${VTK_USE_FILE})

# VTK_Diff実行ファイルを追加
add_executable(VTK_Diff main.cpp)
target_link_libraries(VTK_Diff
  PRIVATE
    ${VTK_LIBRARIES}
    CGAL::CGAL
)


