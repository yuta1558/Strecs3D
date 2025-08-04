# macOS specific settings for Strecs3D

# macOS用のコンパイラフラグ設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# macOS環境でのlib3mf設定
set(LIB3MF_ROOT "$ENV{HOME}/libs/lib3mf/lib3mf-2.3.2-Darwin" CACHE PATH "Path to lib3mf installation")
find_library(LIB3MF_LIB NAMES 3mf PATHS "${LIB3MF_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIB3MF_LIB)
  message(FATAL_ERROR "lib3mf library not found in ${LIB3MF_ROOT}/lib")
else()
  message(STATUS "Found lib3mf library: ${LIB3MF_LIB}")
endif()

# macOS環境でのlibzip設定
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)
message(STATUS "Found libzip version: ${LIBZIP_VERSION}")

if(LIBZIP_LIBRARY_DIRS)
  link_directories(${LIBZIP_LIBRARY_DIRS})
endif()

# macOS用の設定を適用する関数
function(apply_macos_settings TARGET_NAME)
  # macOS用のインクルードパス設定
  target_include_directories(${TARGET_NAME} PRIVATE
    "${LIB3MF_ROOT}/include"
    "${LIB3MF_ROOT}/include/Bindings/Cpp"
    ${LIBZIP_INCLUDE_DIRS}
  )

  # macOS用のリンクライブラリ設定
  target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
    "${LIB3MF_LIB}"
    ${LIBZIP_LIBRARIES}
  )

  # macOSでのQtプラグインパス設定
  set_target_properties(${TARGET_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
  )
endfunction()

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