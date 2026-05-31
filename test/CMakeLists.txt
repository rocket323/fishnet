cmake_minimum_required (VERSION 3.10) 

project(test)

file(GLOB UNIT_SOURCE *.cpp)
include_directories(${GTEST_INCLUDE})
add_executable(${PROJECT_NAME}_test ${UNIT_SOURCE})
target_link_libraries(${PROJECT_NAME}_test ${NET_COMMON_LIB} ${GTEST_ALL_LIB})
