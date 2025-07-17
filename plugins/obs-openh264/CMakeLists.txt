cmake_minimum_required(VERSION 3.28...3.30)

find_package(OpenH264 REQUIRED)

if(NOT TARGET OBS::opts-parser)
  add_subdirectory("${CMAKE_SOURCE_DIR}/shared/opts-parser" "${CMAKE_BINARY_DIR}/shared/opts-parser")
endif()

add_library(obs-openh264 MODULE)
add_library(OBS::openh264 ALIAS obs-openh264)

target_sources(obs-openh264 PRIVATE obs-openh264.c obs-openh264-plugin-main.c)
target_link_libraries(obs-openh264 PRIVATE OBS::opts-parser OpenH264::OpenH264)

if(OS_WINDOWS)
  configure_file(cmake/windows/obs-module.rc.in obs-openh264.rc)
  target_sources(obs-openh264 PRIVATE obs-openh264.rc)
endif()

set_target_properties_obs(obs-openh264 PROPERTIES FOLDER plugins/obs-openh264 PREFIX "")

