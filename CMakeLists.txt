
cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
project("PointCloud")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


add_executable(pointcloud "${CMAKE_CURRENT_SOURCE_DIR}/pointcloud.cpp")
target_link_libraries(pointcloud ${CONAN_LIBS})

