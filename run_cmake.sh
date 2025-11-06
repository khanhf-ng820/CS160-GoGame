#!/bin/bash

rm -rf build/
mkdir build
cd build

cp -r ../assets/ assets/

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

make
