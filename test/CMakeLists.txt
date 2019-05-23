cmake_minimum_required (VERSION 2.8) 

project(test)

file(GLOB UNIT_SOURCE *.cpp)
include_directories(${GTEST_INCLUDE})
add_executable(${PROJECT_NAME}_test ${SOURCE} ${UNIT_SOURCE})
target_link_libraries(${PROJECT_NAME}_test ${NET_COMMON_LIB} ${ALL_LIB})
