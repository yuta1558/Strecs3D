cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Strecs3D LANGUAGES CXX)

# Qt のMOCを自動有効化
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# 未指定の場合はDebugビルドにする（デバッグ情報が付加される）
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debugモードの場合、デバッグ情報を追加する
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# 必要なパッケージの検索
find_package(VTK REQUIRED
  COMPONENTS
    CommonCore
    CommonDataModel
    FiltersCore
    FiltersSources
    FiltersGeometry
    FiltersGeneral
    FiltersModeling
    FiltersExtraction
    FiltersHybrid
    FiltersProgrammable
    FiltersTopology
    FiltersVerdict
    IOXML
    IOImage
    IOGeometry
    IOLegacy
    ImagingCore
    ImagingGeneral
    ImagingHybrid
    ImagingMath
    ImagingSources
    RenderingCore
    RenderingOpenGL2
    RenderingFreeType
    RenderingVolume
    RenderingAnnotation
    RenderingContext2D
    RenderingSceneGraph
    RenderingUI
    RenderingVtkJS
    InteractionStyle
    InteractionWidgets
    GUISupportQt
    GUISupportQtQuick
    GUISupportQtSQL
)

include(${VTK_USE_FILE})

# lib3mf の設定
set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin" CACHE PATH "Path to lib3mf installation")
find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIB3MF_LIB)
  message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
else()
  message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
endif()

# libzip の設定
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)
message(STATUS "Found libzip version: ${LIBZIP_VERSION}")

if(LIBZIP_LIBRARY_DIRS)
  link_directories(${LIBZIP_LIBRARY_DIRS})
endif()

# 実行可能ファイルの生成
add_executable(Strecs3D
  main.cpp
  mainwindow.cpp
  UI/mainwindowui.cpp
  UI/MessageConsole.cpp
  UI/DensitySlider.cpp
  core/VtkProcessor.cpp
  core/lib3mfProcessor.cpp
  utils/fileUtility.cpp
  utils/xmlConverter.cpp
  core/ApplicationController.cpp
  core/FileProcessor.cpp
  core/VisualizationManager.cpp
  core/ExportManager.cpp
)

# インクルードパスの設定
target_include_directories(Strecs3D PRIVATE
  "${LIB3MF_ROOT}/include"
  "${LIB3MF_ROOT}/include/Bindings/Cpp"
  ${LIBZIP_INCLUDE_DIRS}
)

# リンクするライブラリの設定
target_link_libraries(Strecs3D PRIVATE
  Qt6::Core
  Qt6::Widgets
  ${VTK_LIBRARIES}
  "${LIB3MF_LIB}"
  ${LIBZIP_LIBRARIES}
)

# VTK 自動初期化設定 (VTK バージョンが 8.90.0 以上の場合)
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Strecs3D
    MODULES ${VTK_LIBRARIES}
  )
endif()
