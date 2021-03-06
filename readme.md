
# Allolib C/C++ Libraries
Developed by:

AlloSphere Research Group

allolib is a fork of [AlloSystem](https://github.com/AlloSphere-Research-Group/AlloSystem), maintained in separate repository.

University of California, Santa Barbara

[![Build Status](https://travis-ci.org/AlloSphere-Research-Group/allolib.svg?branch=master)](https://travis-ci.org/AlloSphere-Research-Group/allolib)
[![Build status](https://ci.appveyor.com/api/projects/status/c311nw14jmwq9lv1?svg=true)](https://ci.appveyor.com/project/mantaraya36/allolib)

# Documentation

The API documentation can be found at: https://allosphere-research-group.github.io/allolib-doc/

# Installing Allolib

## Dependencies

Allolib only depends on Cmake version 3.8 (as the build tool), OpenGL and glew. See platform specific instructions below.

### Windows

There are two paths for Windows installation. One through Visual Studio and one through the Chocolatey package manager.

For installation through Visual Studio:

 1. Install Visual Studio 2017 Community Edition from https://visualstudio.microsoft.com/downloads/
 2. During installation options:

    a. Install "Desktop development with C++" workload

    b. Install Individual components: C++/CLI support, Git for Windows, Visual C++ Tools for Cmake

For installation through Chocolatey:

 * Install Chocolatey: Aim your browser at https://chocolatey.org/install. Follow the directions there to install Chocolatey. Wait for this to finish before moving on.
 * Use the choco command to install a C++ compiler. Open cmd.exe (Command Prompt) as administrator and run this command: choco install -y visualstudio2017buildtools visualstudio2017-workload-vctools
 * Use the choco command to install some software. Open cmd.exe (Command Prompt) as administrator and run this command: choco install -y git git-lfs cmake graphviz doxygen atom vscode
 * Install libsndfile: Aim your browser at http://www.mega-nerd.com/libsndfile/#Download. Download and install the 64-bit version: libsndfile-1.0.xx-w64-setup.exe.

### macOS

 * Install Xcode: Open the App Store app. Search for "xcode". Install the first result. Wait for this to finish before moving on.
 * Install Homebrew: Open the Terminal app. Aim your browser at https://brew.sh. Copy and paste the text of the install command into the Terminal app.
 * Use the brew command to install some software. In the Terminal app, run this command: brew install git git-lfs cmake libsndfile

### Ubuntu/Debian

    sudo apt install build-essential git git-lfs cmake libsndfile1-dev libassimp-dev

## Building library and running examples
On a bash shell on Windows, Linux and OS X do:

    git clone https://github.com/AlloSphere-Research-Group/allolib
    cd allolib
    git submodule update --recursive --init

To run an example:

    ./run.sh examples/graphics/2D.cpp

# Running allolib

## On Bash shell

The simplest way to compile and run a single file in allolib is by using the run.sh script:

    ./run.sh path/to/file.cpp

This will build allolib, and create an executable for the file.cpp called 'file' inside the '''path/to/bin''' directory. It will then run the application.

You can add a file called '''flags.cmake''' in the '''path/to/''' directory which will be added to the build scripts. Here you can add dependencies, include directories, linking and anything else that cmake could be used for. See the example in '''examples/user_flags'''.

For more complex projects follow the example provided in the empty/project directory. This requires writing a CMakeLists.txt to specify sources, dependencies and linkage.

# TODO

- fullscreen with specific monitor

# Optional Dependencies

- freeimage (optional)
- freetype (optional)
- assimp (optional)

If these are avaialble, the classes that use them will be built. A simple alternative is to use the image/font/asset loading provided in the modeules directory. These classes depend on header only libraries that are included in these sources.

