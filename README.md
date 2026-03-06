# CS160-GoGame
Go Game Final Project for CS160.

This project uses **C++**, **CMake**, and the **SFML 3.0.2** library.

This application uses (most of) the Japanese ruleset and is based on [this Vietnamese article](https://vnchess.com.vn/luat-choi-co-vay-co-ban/) and [this Wikipedia article](https://en.wikipedia.org/wiki/Rules_of_Go).

This application includes a **PVP** mode and a **PVE** mode, which includes an AI opponent that utilizes **minimax** and **alpha-beta pruning** algorithms.

## Building

This project uses **CMake** as its build system.

### 1. Get the prerequisites
Download and make sure you have:
- git (version 2.50.1 or higher)
- [CMake](https://cmake.org/download/) (version 3.16 or higher)
- A C++ compiler (GCC, Clang, MinGW-w64, or MSVC) that supports **C++17**

Make sure all of them are available in PATH in order to be used in the Terminal.

Also make sure you have Internet connection while following these building steps.

Move to **Step 2** if you already downloaded this project using the attached ```.zip``` file in the release notes.

Clone this repository using git:
```bash
git clone --single-branch --branch main https://github.com/khanhf-ng820/CS160-GoGame.git
```
***If you don't have your SSH key set up in Git locally, do this:***
```bash
git config --global url."https://github.com/".insteadOf "git@github.com:"
```
Enter the repository folder and download and update all submodules in the local repository:
```bash
cd CS160-GoGame
git submodule update --init --recursive
```
Verify the status of all submodules:
```bash
git submodule status --recursive
```

### 2. Build the project
Open the Terminal in the **project root folder** and run:

**macOS** and **Linux:**
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

**Windows:**
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
You can run the program by double-clicking the application in the ```build/``` folder inside the project's directory in File Explorer/Finder, or by open the Terminal in the **project root folder** and run:

**macOS** and **Linux:**
```bash
./build/GoGame
```

**Windows:**
```powershell
.\build\GoGame.exe
```
