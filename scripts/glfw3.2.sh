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

# build glfw and install at local site
mkdir -p build
cd build
mkdir -p glfw
cd glfw
cmake -DCMAKE_INSTALL_PREFIX=../.. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF ../../external/glfw
cmake --build . --target install
cd ../..

# copy headers and lib (if not using install target?)
#mkdir -p include
#cp -R external/glfw/include/GLFW include
#mkdir -p lib
#cp build/glfw/src/libglfw3.a lib

