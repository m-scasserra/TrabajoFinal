cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/lib/esp_cron ${CMAKE_SOURCE_DIR}/lib/esp_littlefs)

project(Projecto_Final)
