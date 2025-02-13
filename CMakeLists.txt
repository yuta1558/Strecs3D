cmake_minimum_required(VERSION 3.10)

# プロジェクト宣言 (C++ プロジェクト)
project(Strectus LANGUAGES CXX)

# --- ポリシー設定 ---
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# --- ビルドタイプ設定 ---
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

if(VTK_VERSION VERSION_LESS "8.90.0")
  include(${VTK_USE_FILE})
endif()

# --- lib3mf の設定 ---
set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin" CACHE PATH "Path to lib3mf installation")
find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIB3MF_LIB)
  message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
else()
  message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
endif()

# --- libzip の設定 ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)
message(STATUS "Found libzip version: ${LIBZIP_VERSION}")

# 取得したライブラリディレクトリをリンカに伝える
if(LIBZIP_LIBRARY_DIRS)
  link_directories(${LIBZIP_LIBRARY_DIRS})
endif()

# --- 実行可能ファイルの生成 ---
add_executable(Strectus
  main.cpp
  VtkProcessor.cpp
  cgalProcessor.cpp
  lib3mfProcessor.cpp
  utils/zipUtility.cpp
)

# --- インクルードパスの設定 ---
target_include_directories(Strectus PRIVATE
  "${LIB3MF_ROOT}/include"
  "${LIB3MF_ROOT}/include/Bindings/Cpp"
  ${LIBZIP_INCLUDE_DIRS}
)

# --- リンクするライブラリの設定 ---
target_link_libraries(Strectus PRIVATE
  ${VTK_LIBRARIES}
  CGAL::CGAL
  "${LIB3MF_LIB}"
  ${LIBZIP_LIBRARIES}
)

# --- VTK 自動初期化設定 ---
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Strectus
    MODULES ${VTK_LIBRARIES}
  )
endif()
