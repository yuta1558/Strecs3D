cmake_minimum_required(VERSION 3.10)

# プロジェクト宣言 (C++ プロジェクト)
project(Strectus LANGUAGES CXX)

# --- ポリシー設定 ---
# CMP0167 ポリシーを NEW に設定（FindBoost に関する警告対策）
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# --- ビルドタイプ設定 ---
# CMAKE_BUILD_TYPE が未指定なら Release をデフォルトに設定（CGAL の警告対策）
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# --- C++ 標準設定 ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- パッケージの検索 ---
find_package(VTK REQUIRED)
find_package(CGAL REQUIRED COMPONENTS Core)

# VTK のバージョンに応じた設定
if(VTK_VERSION VERSION_LESS "8.90.0")
  # VTK 8.90 未満では従来の方法を利用
  include(${VTK_USE_FILE})
endif()

# --- lib3mf の設定 ---
# ユーザーが上書きできるように CACHE 変数として設定
set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin" CACHE PATH "Path to lib3mf installation")
find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIB3MF_LIB)
  message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
else()
  message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
endif()

# --- 実行可能ファイルの生成 ---
add_executable(Strectus
  main.cpp
  VtkProcessor.cpp
  cgalProcessor.cpp
  lib3mfProcessor.cpp
)

# --- インクルードパスの設定 ---
target_include_directories(Strectus PRIVATE
  "${LIB3MF_ROOT}/include"
  "${LIB3MF_ROOT}/include/Bindings/Cpp"
)

# --- リンクするライブラリの設定 ---
target_link_libraries(Strectus PRIVATE
  ${VTK_LIBRARIES}
  CGAL::CGAL
  "${LIB3MF_LIB}"
)

# --- VTK 自動初期化設定 ---
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Strectus
    MODULES ${VTK_LIBRARIES}
  )
endif()
