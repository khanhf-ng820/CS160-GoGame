#!/bin/bash

cd build/_deps/flac-src
git add .
git commit -m "Update flac-src to latest"

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
