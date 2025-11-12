#!/bin/bash

LANG=en_US.UTF-8 git submodule update --init --recursive

rm -rf build/
mkdir build
cd build

cp -r ../assets/ assets/

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

make
