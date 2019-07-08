# PointCloud
Point Cloud Generator for Simulation


## Build

The following compilers are tested:

- Clang: Apple LLVM 10.0.0


### Windows & Linux

0. install embree, the conan package can be found [here](https://github.com/KaoCC/conan-packages)
1. Create your build directory `mkdir build && cd build`
2. Run Conan `conan install .. -s cppstd=17 --build missing`
3. Run CMake `cmake ..` for development or `cmake -DCMAKE_BUILD_TYPE=Release` for a release build
4. Compile by running `make`
