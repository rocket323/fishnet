cmake_minimum_required (VERSION 2.8)

project(deps)

#hiredis
add_custom_target(build_hiredis ALL
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${HIREDIS_DIR}/
    COMMENT "build hiredis")
add_library(hiredis STATIC IMPORTED)
set_property(TARGET hiredis APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(hiredis PROPERTIES IMPORTED_LOCATION_NOCONFIG "${HIREDIS_DIR}")
add_dependencies(hiredis build_hiredis)

add_subdirectory(googletest)
