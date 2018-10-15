#!/bin/bash

mkdir -p build
cd build
mkdir -p al
cd al
mkdir -p Debug
cd Debug
cmake ../../.. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
