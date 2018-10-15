#!/bin/bash

mkdir -p build
cd build
mkdir -p al
cd al
mkdir -p Release
cd Release
cmake ../../.. -DCMAKE_BUILD_TYPE=Release
cmake --build .
