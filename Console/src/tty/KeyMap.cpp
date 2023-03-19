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
#ifndef WIN32

#include "Console/tty/KeyMap.h"

namespace _Console
{

std::list<KeyMap> g_keyMap
{
  {K_BS,           "\x08"},
  {K_BS,           "\x7f"},

  {K_UP,           "\x1b[A"},
  {K_DOWN,         "\x1b[B"},
  {K_RIGHT,        "\x1b[C"},
  {K_LEFT,         "\x1b[D"},
  {K_HOME,         "\x1b[H"},
  {K_END,          "\x1b[F"},
  {K_HOME,         "\x1b[1~"},
  {K_END,          "\x1b[4~"},
  {K_PAGEUP,       "\x1b[5~"},
  {K_PAGEDN,       "\x1b[6~"},
  {K_INSERT,       "\x1b[2~"},
  {K_DELETE,       "\x1b[3~"},
  {K_HOME,         "\x1b[7~"},
  {K_END,          "\x1b[8~"},
  {K_END,          "\x1bOF"},

  {K_F1,           "\x1b[[A"},
  {K_F2,           "\x1b[[B"},
  {K_F3,           "\x1b[[C"},
  {K_F4,           "\x1b[[D"},
  {K_F5,           "\x1b[[E"},

  {K_F1,           "\x1b[11~"},
  {K_F2,           "\x1b[12~"},
  {K_F3,           "\x1b[13~"},
  {K_F4,           "\x1b[14~"},
  {K_F5,           "\x1b[15~"},
  {K_F6,           "\x1b[17~"},
  {K_F7,           "\x1b[18~"},
  {K_F8,           "\x1b[19~"},
  {K_F9,           "\x1b[20~"},
  {K_F10,          "\x1b[21~"},

  {K_PAGEUP | K_ALT, "\x1b[5;3~"},
  {K_PAGEDN | K_ALT, "\x1b[6;3~"},

  {K_F1 | K_SHIFT, "\x1bO2P"},
  {K_F2 | K_SHIFT, "\x1bO2Q"},
  {K_F3 | K_SHIFT, "\x1bO2R"},
  {K_F4 | K_SHIFT, "\x1bO2S"},

  //linux XTerm console
  {K_F1 | K_SHIFT, "\x1b[1;2P"},
  {K_F2 | K_SHIFT, "\x1b[1;2Q"},
  {K_F3 | K_SHIFT, "\x1b[1;2R"},
  {K_F4 | K_SHIFT, "\x1b[1;2S"},
  {K_F5 | K_SHIFT, "\x1b[15;2~"},
  {K_F6 | K_SHIFT, "\x1b[17;2~"},
  {K_F7 | K_SHIFT, "\x1b[18;2~"},
  {K_F8 | K_SHIFT, "\x1b[19;2~"},
  {K_F9 | K_SHIFT, "\x1b[20;2~"},
  {K_F10| K_SHIFT, "\x1b[21;2~"},

  //free bsd
  {K_F1 | K_SHIFT, "\x1b[Y"},
  {K_F2 | K_SHIFT, "\x1b[Z"},
  {K_F3 | K_SHIFT, "\x1b[a"},
  {K_F4 | K_SHIFT, "\x1b[b"},
  {K_F5 | K_SHIFT, "\x1b[c"},
  {K_F6 | K_SHIFT, "\x1b[d"},
  {K_F7 | K_SHIFT, "\x1b[e"},
  {K_F8 | K_SHIFT, "\x1b[f"},
  {K_F9 | K_SHIFT, "\x1b[g"},
  {K_F10| K_SHIFT, "\x1b[h"},

  //macx
#ifdef __APPLE__
  {K_F1,           "\x1bOP"},
  {K_F2,           "\x1bOQ"},
  {K_F3,           "\x1bOR"},
  {K_F4,           "\x1bOS"}
#endif
};

//linux terminal default
std::list<KeyMap> g_keyMap1
{
  {K_F11,          "\x1b[23~"},
  {K_F12,          "\x1b[24~"},
  {K_F1 | K_SHIFT, "\x1b[25~"},
  {K_F2 | K_SHIFT, "\x1b[26~"},
  {K_F3 | K_SHIFT, "\x1b[28~"},
  {K_F4 | K_SHIFT, "\x1b[29~"},
  {K_F5 | K_SHIFT, "\x1b[31~"},
  {K_F6 | K_SHIFT, "\x1b[32~"},
  {K_F7 | K_SHIFT, "\x1b[33~"},
  {K_F8 | K_SHIFT, "\x1b[34~"}
};

std::list<KeyMap> g_keyMap2
{
  {K_F1 | K_SHIFT, "\x1b[23~"},
  {K_F2 | K_SHIFT, "\x1b[24~"},
  {K_F3 | K_SHIFT, "\x1b[25~"},
  {K_F4 | K_SHIFT, "\x1b[26~"},
  {K_F5 | K_SHIFT, "\x1b[28~"},
  {K_F6 | K_SHIFT, "\x1b[29~"},
  {K_F7 | K_SHIFT, "\x1b[31~"},
  {K_F8 | K_SHIFT, "\x1b[32~"},
  {K_F9 | K_SHIFT, "\x1b[33~"},
  {K_F10| K_SHIFT, "\x1b[34~"}
};

} //namespace _Console

#endif
