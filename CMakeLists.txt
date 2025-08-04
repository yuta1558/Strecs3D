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

# OS別設定をインクルード（共通設定より前に）
if(WIN32)
  include(cmake/windows_settings.cmake)
elseif(APPLE)
  include(cmake/macos_settings.cmake)
else()
  message(FATAL_ERROR "Unsupported operating system. Only Windows and macOS are supported.")
endif()

# 共通設定をインクルード
include(cmake/common_settings.cmake)
