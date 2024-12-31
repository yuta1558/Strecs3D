cmake_minimum_required(VERSION 3.10)

project(VTK_Diff)

# C++ の標準を設定（必要に応じて変更）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# VTK を見つけて使用する
find_package(VTK REQUIRED)
find_package(CGAL REQUIRED COMPONENTS Core)

if (NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found")
endif()

# VTK の設定を含める
include(${VTK_USE_FILE})

set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin")

# lib3mf のインクルードディレクトリを追加
# モダンな CMake の方法でターゲットごとに設定
# まず、lib3mf のライブラリを検索
find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)

# ライブラリが見つからなかった場合にエラーを出す
if(NOT LIB3MF_LIB)
    message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
else()
    message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
endif()

# VTK_Diff 実行ファイルを追加
add_executable(VTK_Diff main.cpp)

# lib3mf のインクルードディレクトリをターゲットに追加
target_include_directories(VTK_Diff PRIVATE 
    "${LIB3MF_ROOT}/include"
    "${LIB3MF_ROOT}/include/Bindings/Cpp"
)

# ターゲットに必要なライブラリをリンク
target_link_libraries(VTK_Diff
  PRIVATE
    ${VTK_LIBRARIES}
    CGAL::CGAL
    "${LIB3MF_LIB}"
)

# 必要に応じて他のリンクオプションや定義を追加
