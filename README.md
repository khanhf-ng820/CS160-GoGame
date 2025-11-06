# CS160-GoGame
Go Game Final Project for CS160.

This project uses C++, CMake, and SFML library.

## Building

This project uses **CMake** as its build system.

### 1. Get the prerequisites
Make sure you have:
- [CMake](https://cmake.org/download/) (version 3.16 or higher)
- A C++ compiler (GCC, Clang, or MSVC)

### 2. Build the project
Open a terminal in the project folder and run:

```bash
#!/bin/bash
# Create a build folder

rm -rf build/
mkdir build
cd build

cp -r ../assets/ assets/

# Generate build files
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build .
```
