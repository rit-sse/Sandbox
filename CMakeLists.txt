# cmake needs this line
cmake_minimum_required(VERSION 2.8)

# Define project name
project(sandbox)

# Point cmake at modules folder
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

# Glob source
file(GLOB_RECURSE SANDBOX_SOURCES "src/*.cpp")

# Find required packages
find_package(OpenCV REQUIRED)
find_package(libfreenect2 REQUIRED)

set(LIBS ${LIBS} ${freenect2_LIBRARIES})
include_directories(${freenect2_INCLUDE_DIRS})

# Use CPP 11
set(CMAKE_CXX_FLAGS "-std=c++11" ${CMAKE_CXX_FLAGS})

# Declare the executable target built from your sources
add_executable(sandbox ${SANDBOX_SOURCES})
add_executable(box box/box.cpp)
add_executable(depth depth/depth.cpp)
add_executable(hack hack/hack.cpp)

# Link LIBS
target_link_libraries(sandbox ${OpenCV_LIBS})
target_link_libraries(sandbox ${freenect2_LIBRARIES})

target_link_libraries(hack ${OpenCV_LIBS})
target_link_libraries(hack ${freenect2_LIBRARIES})
