# CGPUI SDK consumer sample: configure with
#   cmake -S . -B build -Dcgpui_DIR=<sdk>/lib/cmake/cgpui
#   cmake --build build --config <Debug|Release>
# using the debug package for Debug and the release package for Release.
cmake_minimum_required(VERSION 3.21)
project(cgpui_sdk_consumer LANGUAGES CXX)

find_package(cgpui 0.1 REQUIRED CONFIG)

add_executable(sdk_consumer main.cpp)
target_link_libraries(sdk_consumer PRIVATE cgpui::cgpui)
