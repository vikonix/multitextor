# Multitextor
Cross Platform console mode library and text editor.

This project is mostly recreated version of my old text editor.

## In progress:
- Editor MVP alpha version.
 
## Key features:
- Simple interface same in different environments
- Mouse using
- Multi-window
- Split view mode with 2 panels
- Clear working with different text code pages
- Big files editing over 4 GBytes (with small memory using)
- Deep Undo/Redo

Not MVP (not implemented now):
- Backup files
- Searching in on disk files
- Random access bookmarks
- Editor session saving/restoring
- Customizable interface and key commands
- Customizable syntax highlighting
- Build-in file comparing mode
 
Editor screenshot.
  ![Screenshot](docs/multitextor1.png)

## Tested on:
 - Windows 10 - Microsoft Visual Studio Community 2019 / 2017
 - Windows 7 - Microsoft Visual Studio Community 2017
 - Linux Ubuntu 20.04 - gcc version 9.3.0
 - Armbian Focal OrangePI 4

Need compiler with C++ 0x17 full support.

Minimal requirement: gcc 8.0 or MSVS 2017

[![Build Status](https://travis-ci.org/vikonix/multitextor.svg?branch=main)][travis]
[![BSD-2 license](https://img.shields.io/github/license/vikonix/multitextor)][license]

[travis]: https://travis-ci.org/vikonix/multitextor
[license]: https://github.com/vikonix/multitextor/blob/main/LICENSE

## Need to install packages in Linux:
 - sudo apt-get install -y libncurses5-dev
 - sudo apt-get install -y libgpm-dev
 - sudo apt-get install -y gpm (only for mouse supporting in console)
 
## How to build
 - Install CMake 3.15 or higher
 - Install g++ or clang or MSVC
 - Run CMake: **cmake -B _build -S .**
    or **cmakegen.bat**
 - Build editor
    in Linux run: **build.sh**
    in Windows try to run: **msbuild /p:Configuration=Release BigProject.sln**
    or open solution _build/Multitextor.sln with MSVC
 - Get editor in Linux _build/bin/multitextor or in Windows _build/bir/Debug|Release/multitextor.exe
    
 
