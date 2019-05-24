cmake_minimum_required (VERSION 2.8) 
project(fishnet CXX)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fPIC -std=c++11 -Wno-literal-suffix -Wno-error=literal-suffix -Wno-deprecated -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing -fno-strict-aliasing -DNDEBUG")

SET(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(DEPS_DIR ${CMAKE_ROOT_DIR}/deps)

# googletest
SET(GTEST_DIR ${DEPS_DIR}/googletest/googletest)
SET(GTEST_INCLUDE ${GTEST_DIR}/include)
SET(GTEST_LIB_DIR ${CMAKE_ROOT_DIR}/build/lib/)
SET(GTEST_ALL_LIB ${GTEST_LIB_DIR}/libgtest_main.a ${GTEST_LIB_DIR}/libgtest.a)

# hiredis
SET(HIREDIS_DIR ${DEPS_DIR}/hiredis/)

file(GLOB SOURCE *.cpp tcp/*.cpp redis/*.cpp)

SET(NET_COMMON_INCLUDE
    ${CMAKE_ROOT_DIR}
    ${HIREDIS_DIR}
    )

SET(NET_COMMON_LIB
    ${CMAKE_ROOT_DIR}/libfishnet.a
    ${HIREDIS_DIR}/libhiredis.a
    pthread
    )

SET(NET_COMMON_DEPENDENCIES fishnet build_hiredis)

include_directories(${NET_COMMON_INCLUDE})
add_library(${PROJECT_NAME} ${SOURCE})
set(LIBRARY_OUTPUT_PATH ${CMAKE_ROOT_DIR})

add_subdirectory(deps)
add_subdirectory(test)
add_subdirectory(examples)
