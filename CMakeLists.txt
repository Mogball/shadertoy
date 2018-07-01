# Variables
set(TARGET shadertoy)

# Project setup
cmake_minimum_required(VERSION 3.1.0)
project(${TARGET} C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
set(CMAKE_BUILD_TYPE Debug)

# Target configuration
file(GLOB_RECURSE FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

add_executable(${TARGET} ${FILES})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

# GLSL compilation
#add_custom_target(GLSL_COMPILE COMMAND ${CMAKE_CURRENT_LIST_DIR}/glsl_compile.sh)
#add_dependencies(${TARGET} GLSL_COMPILE)

# Find SFML
find_package(SFML 2 REQUIRED graphics window system)
if (SFML_FOUND)
    target_include_directories(${TARGET} PRIVATE ${SFML_INCLUDE_DIR})
    target_link_libraries(${TARGET} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
else()
    message(FATAL_ERROR "Failed to find SFML")
endif()

# Find GLEW
#find_package(GLEW REQUIRED)
#if (GLEW_FOUND)
#    target_include_directories(${TARGET} PRIVATE ${GLEW_INCLUDE_DIR})
#    target_link_libraries(${TARGET} ${GLEW_LIBRARIES})
#else()
#    message(FATAL_ERROR "Failed to find GLEW")
#endif()

# Find OpenGL
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    target_include_directories(${TARGET} PRIVATE ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${TARGET} ${OPENGL_LIBRARIES})
else()
    message(FATAL_ERROR "Failed to find OpenGL")
endif()
