# PointCloud
Point Cloud Generator for Simulation


## Dependency

- [CMake 3.9+](https://cmake.org/)
- [Conan 1.20.3+](https://conan.io/)
- [embree/3.6.1@kaocc/stable](https://github.com/KaoCC/conan-packages/tree/master/recipes/embree)

## Build

The following compilers are tested:

- Clang: Apple LLVM 10.0.0, 11.0.0

### Windows & Linux

0. install embree, the conan package can be found [here](https://github.com/KaoCC/conan-packages)
1. Create your build directory `mkdir build && cd build`
2. Run Conan `conan install .. --build missing`
3. Run CMake `cmake ..` for development or `cmake -DCMAKE_BUILD_TYPE=Release` for a release build
4. Compile by running `make`
