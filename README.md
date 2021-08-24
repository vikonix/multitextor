# Multitextor
![Screenshot](docs/m.png) Cross platform console mode text editor.

This project is mostly recreated version of my old text editor.

[![BSD-2 license](https://img.shields.io/github/license/vikonix/multitextor)](https://github.com/vikonix/multitextor/blob/main/LICENSE)

### In progress
- Editor 2.0.0-beta version.
 
### Key features
- Simple user friendly interface same in different environments (with menu and dialog)
- Mouse and keyboard cursor movement and selection
- Multi-window
- Split view mode with 2 panels
- Clear working with different text code pages
- Different select modes
- Working with macros
- Big files editing over 4 GBytes (with small memory using)
- Deep Undo/Redo
- Customizable key commands and some interface parameters
- Customizable syntax highlighting
- Editor session saving/restoring
- Searching in on disk files

Will be implemented in the next versions:
- Backup files
- Random access bookmarks
- Build-in file comparing mode
 
Editor screenshot.
  ![Screenshot](docs/multitextor1.png)

### Tested on
Linux/Windows/OSX build (Travis CI): [![Build Status](https://travis-ci.com/vikonix/multitextor.svg?branch=main)](https://travis-ci.com/vikonix/multitextor)

 - Windows 10 - Microsoft Visual Studio Community 2019 / 2017
 - Windows 7 - Microsoft Visual Studio Community 2017
 - Linux Ubuntu 18.04 - gcc version 9.3.0
 - Linux Ubuntu 20.04 - gcc version 9.3.0
 - Armbian Focal OrangePI 4

For building it needs a compiler with C++ 0x17 full support.

Minimal requirement: gcc 8.0 or MSVS 2017

### Need to install packages in Linux
 - sudo apt-get install -y libncurses5-dev
 - sudo apt-get install -y libgpm-dev
 - sudo apt-get install -y gpm (only for mouse supporting in console)
 
### How to build
 - Install CMake 3.15 or higher
 - Install g++-9 or clang or MSVC
 - Run CMake: ***cmake -B _build -S .***
 
    or ***cmakegen.bat***
    
 - Build editor
    - in Linux run: ***build.sh***
    - in Windows try to run: ***msbuild /p:Configuration=Release Multitextor.sln*** 
    - or open solution ***_build/Multitextor.sln*** with MSVC
    
 - Get editor in Linux ***_build/bin/multitextor*** or in Windows ***_build/bin/Debug|Release/multitextor.exe***
    
### Linux: get binaries packet from snap
Snap packet link: [![snap packet](https://snapcraft.io/multitextor/badge.svg)](https://snapcraft.io/multitextor)

 - Install:
    ***snap install --edge --devmode multitextor***

 - Update: 
    ***snap refresh --edge --devmode multitextor***
    
### Windows: get zip archive from AppVeyor CI artifacts
Zip archive link: [![zip archive](https://ci.appveyor.com/api/projects/status/m98q8sh347k0cdu6/branch/main?svg=true)](https://ci.appveyor.com/project/vikonix/multitextor/branch/main/artifacts)
