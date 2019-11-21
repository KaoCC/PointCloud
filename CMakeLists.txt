
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("PointCloud" LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


set(BASE_SRC

    src/pointcloud.cpp
    src/helper.cpp
)


add_executable(pointcloud ${BASE_SRC})

if(MSVC)
  target_compile_options(pointcloud PRIVATE /W4 /WX)
else()
  target_compile_options(pointcloud PRIVATE -Wall -Wextra -pedantic)
endif()


target_link_libraries(pointcloud ${CONAN_LIBS})
target_compile_features(pointcloud PUBLIC cxx_std_17)
