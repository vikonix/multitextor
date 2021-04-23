# Multitextor
Cross Platform console mode library and text editor.

This project is fully recreated version of my old text editor.

## Key features:
- Simple interface same in difference environments
- Multi-window
- Customizable
- Clear working with different text code pages
- Big files editing over 4 GBytes (with small memory using)
- Deep Undo/Redo

Not MVP (not implemented now):
- Backup files
- Searching in on disk files
- Random access bookmarks
- Editor session saving/restoring
- Customisable syntax higlighting
- Build-in file comparing mode
 
## Tested on:
 - Windows 7 - Microsoft Visual Studio Community 2017
 - Windows 10 - Microsoft Visual Studio Community 2019 / 2017
 - Linux Ubuntu 20.04 - gcc version 9.3.0

Need compiler with C++ 0x17 full support.

Minimal requirement: gcc 8.0 or MSVS 2017

[![Build Status](https://travis-ci.org/vikonix/multitextor.svg?branch=main)][travis]
[![BSD-2 license](https://img.shields.io/github/license/vikonix/multitextor)][license]

[travis]: https://travis-ci.org/vikonix/multitextor
[license]: https://github.com/vikonix/multitextor/blob/main/LICENSE

## Need to install packages in Linux:
 - sudo apt-get install -y libncurses5-dev
 - sudo apt-get install -y libgpm-dev
 - sudo apt-get install -y gpm
 
## Used third-party libraries:
 - easyloggingpp
 - termdb
 - utfcpp
 - win-iconv
 - iconv
 
## In progress:
 - Editor MVP alpha version.