
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("PointCloud" LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()



add_executable(pointcloud "${CMAKE_CURRENT_SOURCE_DIR}/pointcloud.cpp")

if(MSVC)
  target_compile_options(pointcloud PRIVATE /W4 /WX)
else()
  target_compile_options(pointcloud PRIVATE -Wall -Wextra -pedantic -Werror)
endif()


target_link_libraries(pointcloud ${CONAN_LIBS})
target_compile_features(pointcloud PUBLIC cxx_std_17)
