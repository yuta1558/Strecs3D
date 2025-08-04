# Common settings for Strecs3D

# 未指定の場合はDebugビルドにする（デバッグ情報が付加される）
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# OS別の設定を適用
if(WIN32)
  apply_windows_settings(Strecs3D)
elseif(APPLE)
  apply_macos_settings(Strecs3D)
endif()

# VTK 自動初期化設定 (VTK バージョンが 8.90.0 以上の場合)
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Strecs3D
    MODULES ${VTK_LIBRARIES}
  )
endif() 