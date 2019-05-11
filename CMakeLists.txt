cmake_minimum_required (VERSION 2.8)

project(fishnet)

# file(GLOB_RECURSE SOURCE main.cpp)
set(SOURCE main.cpp)

include_directories(${PROJECT_SOURCE_DIR})
add_executable(${PROJECT_NAME} ${SOURCE})
