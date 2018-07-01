set(TARGET shadertoy)

cmake_minimum_required(VERSION 3.1.0)
project(${TARGET})

file(GLOB_RECURSE FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

add_executable(${TARGET} ${FILES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
find_package(SFML 2 REQUIRED graphics window system)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${TARGET} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
else()
    message(FATAL_ERROR "Failed to find SFML")
endif()

