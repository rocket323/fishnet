cmake_minimum_required (VERSION 2.8) 

project(examples)

include_directories(${PROJECT_SOURCE_DIR} ${NET_COMMON_INCLUDE})
link_libraries(${NET_COMMON_LIB})

add_executable(echo_client echo_client.cpp)
add_dependencies(echo_client ${NET_COMMON_DEPENDENCIES})

add_executable(echo_server echo_server.cpp)
add_dependencies(echo_server ${NET_COMMON_DEPENDENCIES})

add_executable(ping_pong_client ping_pong_client.cpp)
add_dependencies(ping_pong_client ${NET_COMMON_DEPENDENCIES})

add_executable(ping_pong_server ping_pong_server.cpp)
add_dependencies(ping_pong_server ${NET_COMMON_DEPENDENCIES})

add_executable(redis_client redis_client.cpp)
add_dependencies(redis_client ${NET_COMMON_DEPENDENCIES})
