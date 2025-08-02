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
  if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  endif()
endif()

# Windows環境でのvcpkgサポート
if(WIN32)
  # vcpkgのツールチェーンファイルを検索
  if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
  endif()
  
  # Windows用のコンパイラフラグ設定
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  endif()
endif()

# Qt6の検索
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Qt6の詳細設定
if(Qt6_FOUND)
  message(STATUS "Qt6 found: ${Qt6_VERSION}")
  message(STATUS "Qt6 Core: ${Qt6Core_DIR}")
  message(STATUS "Qt6 Widgets: ${Qt6Widgets_DIR}")
else()
  message(FATAL_ERROR "Qt6 not found. Please install Qt6 development libraries.")
endif()

# MinGW環境でのQt6設定
if(MINGW)
  # MinGW用のQt6設定
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DEPRECATED_WARNINGS")
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

# lib3mf の設定 - プラットフォーム別
if(WIN32)
  # Windows環境でのlib3mf設定
  set(LIB3MF_ROOT "$ENV{USERPROFILE}/libs/lib3mf/lib3mf-2.3.2-Windows" CACHE PATH "Path to lib3mf installation")
  find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" "${LIB3MF_ROOT}/lib/x64" NO_DEFAULT_PATH)
  if(NOT LIB3MF_LIB)
    # vcpkgからlib3mfを検索
    find_package(lib3mf CONFIG QUIET)
    if(lib3mf_FOUND)
      message(STATUS "Found lib3mf via vcpkg")
      set(LIB3MF_LIB lib3mf::lib3mf)
    else()
      message(FATAL_ERROR "lib3mf library not found. Please install via vcpkg: vcpkg install lib3mf")
    endif()
  else()
    message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
  endif()
else()
  # macOS環境でのlib3mf設定
  set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin" CACHE PATH "Path to lib3mf installation")
  find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)
  if(NOT LIB3MF_LIB)
    message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
  else()
    message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
  endif()
endif()

# libzip の設定 - プラットフォーム別
if(WIN32)
  # Windows環境でのlibzip設定
  find_package(libzip CONFIG QUIET)
  if(libzip_FOUND)
    message(STATUS "Found libzip via vcpkg")
    set(LIBZIP_LIBRARIES libzip::zip)
    set(LIBZIP_INCLUDE_DIRS "")
  else()
    # 従来の方法で検索
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED libzip)
    message(STATUS "Found libzip version: ${LIBZIP_VERSION}")
    
    if(LIBZIP_LIBRARY_DIRS)
      link_directories(${LIBZIP_LIBRARY_DIRS})
    endif()
  endif()
else()
  # macOS環境でのlibzip設定
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBZIP REQUIRED libzip)
  message(STATUS "Found libzip version: ${LIBZIP_VERSION}")
  
  if(LIBZIP_LIBRARY_DIRS)
    link_directories(${LIBZIP_LIBRARY_DIRS})
  endif()
endif()

# 実行可能ファイルの生成
add_executable(Strecs3D
  main.cpp
  mainwindow.cpp
  UI/mainwindowui.cpp
  UI/MessageConsole.cpp
  UI/DensitySlider.cpp
  UI/ColorManager.cpp
  UI/Button.cpp
  UI/ModeComboBox.cpp
  UI/ObjectDisplayOptionsWidget.cpp
  UI/DisplayOptionsContainer.cpp
  UI/CustomCheckBox.cpp
  core/VtkProcessor.cpp
  core/lib3mfProcessor.cpp
  utils/fileUtility.cpp
  utils/tempPathUtility.cpp
  utils/xmlConverter.cpp
  core/ApplicationController.cpp
  core/ProcessPipeline.cpp
  core/VisualizationManager.cpp
  core/ExportManager.cpp
  resources/resources.qrc
)

# インクルードパスの設定 - プラットフォーム別
if(WIN32)
  if(NOT lib3mf_FOUND)
    target_include_directories(Strecs3D PRIVATE
      "${LIB3MF_ROOT}/include"
      "${LIB3MF_ROOT}/include/Bindings/Cpp"
    )
  endif()
  if(NOT libzip_FOUND)
    target_include_directories(Strecs3D PRIVATE
      ${LIBZIP_INCLUDE_DIRS}
    )
  endif()
else()
  target_include_directories(Strecs3D PRIVATE
    "${LIB3MF_ROOT}/include"
    "${LIB3MF_ROOT}/include/Bindings/Cpp"
    ${LIBZIP_INCLUDE_DIRS}
  )
endif()

# リンクするライブラリの設定 - プラットフォーム別
if(WIN32)
  target_link_libraries(Strecs3D PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
    ${LIB3MF_LIB}
    ${LIBZIP_LIBRARIES}
  )
  
  # MinGW環境での追加設定
  if(MINGW)
    # MinGW用のQt6ライブラリパス設定
    if(Qt6Core_DIR)
      get_filename_component(QT_LIB_DIR "${Qt6Core_DIR}/../" ABSOLUTE)
      message(STATUS "Qt6 library directory: ${QT_LIB_DIR}")
    endif()
    
    # MinGW用のリンクフラグ
    set_target_properties(Strecs3D PROPERTIES
      LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
  endif()
else()
  target_link_libraries(Strecs3D PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
    "${LIB3MF_LIB}"
    ${LIBZIP_LIBRARIES}
  )
endif()

# VTK 自動初期化設定 (VTK バージョンが 8.90.0 以上の場合)
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Strecs3D
    MODULES ${VTK_LIBRARIES}
  )
endif()

# プラットフォーム別の設定
if(WIN32)
  # Windows環境での設定
  set_target_properties(Strecs3D PROPERTIES
    WIN32_EXECUTABLE TRUE
    VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/$<CONFIG>;$ENV{PATH}"
  )
  
  # Windows用のリソースファイル設定（必要に応じて）
  # set_target_properties(Strecs3D PROPERTIES
  #   WIN32_EXECUTABLE TRUE
  #   VS_GLOBAL_ROOTNAMESPACE "Strecs3D"
  # )
  
  # Qtプラグインパスの設定
  if(Qt6_DIR)
    get_filename_component(QT_ROOT "${Qt6_DIR}/../../../" ABSOLUTE)
    set(QT_PLUGIN_PATH "${QT_ROOT}/plugins")
    message(STATUS "Qt plugins path: ${QT_PLUGIN_PATH}")
  endif()
  
elseif(APPLE)
  # macOSでのQtプラグインパス設定
  # Qtのプラグインパスを環境変数として設定
  set_target_properties(Strecs3D PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
  )
  
  # Qtプラグインパスを設定
  set(QT_PLUGIN_PATH "/opt/homebrew/Cellar/qt/6.9.1/share/qt/plugins")
  if(EXISTS "${QT_PLUGIN_PATH}")
    message(STATUS "Found Qt plugins at: ${QT_PLUGIN_PATH}")
  else()
    # 代替パスを試す
    set(QT_PLUGIN_PATH "/opt/homebrew/share/qt/plugins")
    if(EXISTS "${QT_PLUGIN_PATH}")
      message(STATUS "Found Qt plugins at: ${QT_PLUGIN_PATH}")
    else()
      message(WARNING "Qt plugins not found. Please set QT_PLUGIN_PATH manually.")
    endif()
  endif()
endif()
