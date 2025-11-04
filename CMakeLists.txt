cmake_minimum_required(VERSION 3.10)

project(balboa)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

if(MSVC)
  add_compile_options(/Wall)
else()
  add_compile_options(-Wall)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SRCS src/3rdparty/glad.h
         src/3rdparty/json.hpp
         src/3rdparty/khrplatform.h
         src/3rdparty/stb_image.h
         src/3rdparty/stb_image_write.h
         src/3rdparty/tinyply.h
         src/flexception.h
         src/3rdparty/glad.c
         src/hw1.h
         src/hw1_scenes.h
         src/hw2.h
         src/hw2_scenes.h
         src/hw3.h
         src/hw3_scenes.h
         src/image.h
         src/matrix.h
         src/timer.h
         src/vector.h
         src/3rdparty/tinyply.cpp
         src/hw1.cpp
         src/hw1_scenes.cpp
         src/hw2.cpp
         src/hw2_scenes.cpp
         src/hw3.cpp
         src/hw3_scenes.cpp
         src/image.cpp)

add_library(balboa_lib STATIC ${SRCS})
add_executable(balboa src/main.cpp)

# GLFW settings
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(src/3rdparty/glfw)

# OpenGL
find_package(OpenGL REQUIRED)

# OpenMP
if(APPLE)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_C_FLAGS "${OpenMP_C_FLAGS} -I/opt/homebrew/opt/libomp/include")
        set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS} -I/opt/homebrew/opt/libomp/include")
        set(OpenMP_omp_LIBRARY "/opt/homebrew/opt/libomp/lib/libomp.dylib")
    endif()
endif()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(balboa_lib PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found and enabled for multithreading")
else()
    message(WARNING "OpenMP not found.")
endif()

target_link_libraries(balboa balboa_lib glfw OpenGL::GL)
