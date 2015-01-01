cmake_minimum_required(VERSION 3.0)

project(qautozen)

# Qt stuff
find_package(Qt5Widgets)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Portaudio stuff
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)
find_library(PORTAUDIO_LIB portaudio)
include_directories(${PORTAUDIO_INCLUDE_DIR})

# Our stuff
list(APPEND SOURCES main.cpp file_processor.cpp main_window.cpp sound_thread_portaudio.cpp)

add_executable(qautozen WIN32 MACOSX_BUNDLE ${SOURCES})
target_compile_options(qautozen PRIVATE -std=c++11)
target_link_libraries(qautozen Qt5::Widgets ${PORTAUDIO_LIB})

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(VER "${VER_MAJOR}.${VER_MINOR}.${VER_PATCH}")

set_target_properties(qautozen PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Autozen"
  MACOSX_BUNDLE_BUNDLE_VERSION ${VER})

if (APPLE)
  target_compile_options(qautozen PRIVATE -stdlib=libc++)

  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_PACKAGE_VERSION ${VER})
  set(CPACK_PACKAGE_VERSION_MAJOR ${VER_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${VER_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${VER_PATCH})
  set(CPACK_PACKAGE_FILE_NAME "qautozen-${VER}")
  include(CPack)
  set(CMAKE_INSTALL_PREFIX "/Applications")
  install(TARGETS qautozen
    DESTINATION .)
endif (APPLE)
