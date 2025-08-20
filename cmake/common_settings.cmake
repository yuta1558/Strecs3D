# Common settings for Strecs3D

# 未指定の場合はDebugビルドにする（デバッグ情報が付加される）
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkgの設定確認
if(DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
else()
  message(WARNING "vcpkg toolchain not found. Please set VCPKG_ROOT environment variable.")
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

# vcpkgからlibzipを検索
find_package(libzip REQUIRED)
if(libzip_FOUND)
  message(STATUS "libzip found: ${libzip_VERSION}")
else()
  message(FATAL_ERROR "libzip not found. Please install via vcpkg: vcpkg install libzip")
endif()

# vcpkgからlib3mfを検索
find_package(lib3mf REQUIRED)
if(lib3mf_FOUND)
  message(STATUS "lib3mf found: ${lib3mf_VERSION}")
else()
  message(FATAL_ERROR "lib3mf not found. Please install via vcpkg: vcpkg install lib3mf")
endif()

# vcpkgからVTKを検索（Qtサポート付き）
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
    GUISupportQtSQL
)

if(VTK_FOUND)
  message(STATUS "VTK found: ${VTK_VERSION}")
  include(${VTK_USE_FILE})
else()
  message(FATAL_ERROR "VTK not found. Please install via vcpkg: vcpkg install vtk[qt]")
endif()

# 実行可能ファイルの生成
add_executable(Strecs3D
  main.cpp
  mainwindow.cpp
  UI/mainwindowui.cpp
  UI/widgets/MessageConsole.cpp
  UI/widgets/DensitySlider.cpp
  UI/ColorManager.cpp
  UI/widgets/Button.cpp
  UI/widgets/ModeComboBox.cpp
  UI/widgets/ObjectDisplayOptionsWidget.cpp
  UI/widgets/DisplayOptionsContainer.cpp
  UI/widgets/CustomCheckBox.cpp
  UI/SceneRenderer.cpp
  core/processing/VtkProcessor.cpp
  core/processing/lib3mfProcessor.cpp
  utils/fileUtility.cpp
  utils/tempPathUtility.cpp
  utils/xmlConverter.cpp
  core/application/ApplicationController.cpp
  core/application/MainWindowUIAdapter.cpp
  core/interfaces/IUserInterface.cpp
  core/processing/ProcessPipeline.cpp
  core/visualization/VisualizationManager.cpp
  core/visualization/SceneDataController.cpp
  core/export/ExportManager.cpp
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

# -----------------------
# ここからインストール設定
# -----------------------
include(GNUInstallDirs)

# 配置場所の規約
set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_LIBDIR lib)

# Strecs3D バイナリをインストール
install(TARGETS Strecs3D
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # Windows .exe
  BUNDLE  DESTINATION .                        # macOS .app
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 追加のリソースがあれば必要に応じて（例）
# install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_BINDIR}/assets)
# install(FILES config/app.ini DESTINATION ${CMAKE_INSTALL_BINDIR})
