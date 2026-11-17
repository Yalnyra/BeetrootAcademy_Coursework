# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Build only main + what it depends on (skips esp_lcd, wifi, bt, ...)
# set(COMPONENTS lesson_4.1_uart)
set(COMPONENTS lesson_4.2_i2c)

# Tell ESP-IDF to also look inside ESP32/ for components (finds ESP32/main/)
set(EXTRA_COMPONENT_DIRS "ESP32")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(4_Interfaces)
