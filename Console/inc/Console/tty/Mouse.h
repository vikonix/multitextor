/*
FreeBSD License

Copyright (c) 2020-2023 vikonix: valeriy.kovalev.software@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#define USE_MOUSE
#ifdef USE_MOUSE
  #include "Console/Types.h"

  #define InitMouse()   _InitMouse()
  #define DeinitMouse() _DeinitMouse()
  #define GetMouseFD()  _GetMouseFD()
  #define ReadMouse()   _ReadMouse()

namespace _Console
{

bool    _InitMouse();
bool    _DeinitMouse();
int     _GetMouseFD();
input_t _ReadMouse();

} //namespace _Console

  //#define OLD_MOUSE //define in case "SGR extended mouse reporting" not working

#else
  #define InitMouse()   ((void) false)
  #define DeinitMouse() ((void) false)
  #define GetMouseFD()  -1
  #define ReadMouse()   0
#endif
