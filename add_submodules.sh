#!/bin/bash

export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8 # LC_ALL overrides other LC_* variables

cd build/_deps/flac-src
LANG=en_US.UTF-8 git add .
LANG=en_US.UTF-8 git commit -m "Update flac-src to latest"

cd ../freetype-src
git add .
git commit -m "Update freetype-src to latest"

cd ../ogg-src
git add .
git commit -m "Update ogg-src to latest"

cd ../vorbis-src
git add .
git commit -m "Update vorbis-src to latest"

cd ../../..
