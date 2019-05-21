cmake_minimum_required (VERSION 2.8) 
project(fishnet CXX)

set(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DEPS_DIR ${CMAKE_ROOT_DIR}/deps)

# googletest
set(GTEST_DIR ${DEPS_DIR}/googletest/googletest)
set(GTEST_INCLUDE ${GTEST_DIR}/include)
set(GTEST_LIB_DIR ${CMAKE_ROOT_DIR}/build/lib/)
set(GTEST_ALL_LIB ${GTEST_LIB_DIR}/libgtest.a ${GTEST_LIB_DIR}/libgtest_main.a)

set(ALL_LIB ${GTEST_ALL_LIB} pthread)

file(GLOB SOURCE *.cpp tcp/*.cpp)

SET(NET_COMMON_INCLUDE
    ${CMAKE_ROOT_DIR}
    )

SET(NET_COMMON_LIB
    ${CMAKE_ROOT_DIR}/libfishnet.a
    )

include_directories(${CMAKE_ROOT_DIR})
add_library(${PROJECT_NAME} ${SOURCE})
set(LIBRARY_OUTPUT_PATH ${CMAKE_ROOT_DIR})

add_subdirectory(deps)
add_subdirectory(test)
add_subdirectory(examples)
