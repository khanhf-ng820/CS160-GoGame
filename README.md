# CS160-GoGame
Go Game Final Project for CS160.

This project uses C++, CMake, and the SFML 3.0.2 library.

This application uses the Japanese ruleset and is based on [this Vietnamese article](https://vnchess.com.vn/luat-choi-co-vay-co-ban/) and [this Wikipedia article](https://en.wikipedia.org/wiki/Rules_of_Go).

## Building

This project uses **CMake** as its build system.

### 1. Get the prerequisites
Make sure you have:
- git (version 2.50.1 or higher)
- [CMake](https://cmake.org/download/) (version 3.16 or higher)
- A C++ compiler (GCC, Clang, MinGW-w64, or MSVC)

Make sure all of them are available in PATH.

Clone this repository using git:
```bash
git clone --single-branch --branch prototype https://github.com/khanhf-ng820/CS160-GoGame.git
```
Download and update all submodules in the local repository:
```bash
git submodule update --init --recursive
```
Verify the status of all submodules:
```bash
git submodule status --recursive
```

### 2. Build the project
Open a terminal in the **project root folder** and run:

macOS:
```bash
#!/bin/bash

# Create a build folder
mkdir build
cd build

# Generate build files
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B .

# Build the project
cmake --build .
```

Windows:
```bash
#!/bin/bash

# Create a build folder
mkdir build
cd build

# Generate build files
# If you use the MinGW C++ compiler, do this:
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -S .. -B .
# Else, do this:
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B .

# Build the project
cmake --build .
```

### 3. Running the program
Open a terminal in the **project root folder** and run:

macOS:
```bash
./build/GoGame
```

Windows:
```bash
.\build\GoGame.exe
```
