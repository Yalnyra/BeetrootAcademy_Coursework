# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Build only main + what it depends on (skips esp_lcd, wifi, bt, ...)
set(COMPONENTS main)

# Reusable driver components (encoder, button, servo, buzzer).
# ESP-IDF also scans <project>/components by default; listing it here makes the
# source folder explicit and is the line to change if the folder ever moves.
list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(3_Module_ADC)
