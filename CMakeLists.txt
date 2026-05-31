cmake_minimum_required (VERSION 3.10) 
project(fishnet CXX)

# Suppress deprecation warnings from third-party dependencies (e.g., googletest)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fPIC -Wno-deprecated -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing -fno-strict-aliasing -DNDEBUG")

SET(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(DEPS_DIR ${CMAKE_ROOT_DIR}/deps)

# googletest
SET(GTEST_DIR ${DEPS_DIR}/googletest/googletest)
SET(GTEST_INCLUDE ${GTEST_DIR}/include)
SET(GTEST_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
# Use target names instead of absolute paths where possible
SET(GTEST_ALL_LIB gtest_main gtest)

# hiredis
SET(HIREDIS_DIR ${DEPS_DIR}/hiredis/)

# protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

file(GLOB SOURCE *.cpp net/*.cpp net/tcp/*.cpp redis2/*.cpp rpc/*.cpp)
list(REMOVE_ITEM SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/net/epoll_poller.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/net/select_poller.cpp")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS rpc/rpc.proto)
list(APPEND SOURCE ${PROTO_SRCS} ${PROTO_HDRS})

SET(NET_COMMON_INCLUDE
    ${CMAKE_ROOT_DIR}
    ${CMAKE_ROOT_DIR}/net
    ${CMAKE_ROOT_DIR}/net/tcp
    ${HIREDIS_DIR}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

# Use target name 'fishnet' instead of hardcoded file path
SET(NET_COMMON_LIB
    fishnet
    ${HIREDIS_DIR}/libhiredis.a
    ${PROTOBUF_LIBRARIES}
    pthread
    )

SET(NET_COMMON_DEPENDENCIES fishnet build_hiredis)

include_directories(${NET_COMMON_INCLUDE})
add_library(${PROJECT_NAME} ${SOURCE})
set(LIBRARY_OUTPUT_PATH ${CMAKE_ROOT_DIR})

add_subdirectory(deps)
add_subdirectory(test)
add_subdirectory(examples)
