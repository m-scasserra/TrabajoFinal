# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources})
#idf_component_register(SRCS "led_strip_example_main.c" "led_strip_encoder.c"
#                       INCLUDE_DIRS ".")