#!/bin/bash

# clone glfw if not cloned yet
# commit 999f355 on 2016-08-18: version 3.2.1
mkdir -p external
cd external
if [ ! -d "glfw" ]; then
    git clone --shallow-since=2016-08-18 https://github.com/glfw/glfw.git
fi
cd glfw
git checkout 999f355
cd ../..

# build glfw
mkdir -p build
cd build
mkdir -p glfw
cd glfw
cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF ../../external/glfw
cmake --build .
cd ../..

# copy headers
mkdir -p include
rm -rf include/GLFW
cp -R external/glfw/include/GLFW include

# copy lib
mkdir -p libs
cp build/glfw/src/libglfw3.a libs

