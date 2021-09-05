/*
FreeBSD License

Copyright (c) 2020-2021 vikonix: valeriy.kovalev.software@gmail.com
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

#define TERMDB_IMPLEMENTATION
#include "Console/tty/TermcapMap.h"
#include "Console/KeyCodes.h"


//////////////////////////////////////////////////////////////////////////////
namespace _Console
{

std::list<KeyCap> g_keyCap
{
  {"kb", K_BS},

  {"ku", K_UP},
  {"kd", K_DOWN},
  {"kr", K_RIGHT},
  {"kl", K_LEFT},
  {"kh", K_HOME},
  {"kH", K_END},
  {"kP", K_PAGEUP},
  {"kN", K_PAGEDN},
  {"kI", K_INSERT},
  {"kD", K_DELETE},

  {"k1", K_F1},
  {"k2", K_F2},
  {"k3", K_F3},
  {"k4", K_F4},
  {"k5", K_F5},
  {"k6", K_F6},
  {"k7", K_F7},
  {"k8", K_F8},
  {"k9", K_F9},
  {"k;", K_F10},
  {"F1", K_F1  | K_SHIFT},
  {"F2", K_F2  | K_SHIFT},
  {"F3", K_F3  | K_SHIFT},
  {"F4", K_F4  | K_SHIFT},
  {"F5", K_F5  | K_SHIFT},
  {"F6", K_F6  | K_SHIFT},
  {"F7", K_F7  | K_SHIFT},
  {"F8", K_F8  | K_SHIFT},
  {"F9", K_F9  | K_SHIFT},
  {"FA", K_F10 | K_SHIFT}
};


//////////////////////////////////////////////////////////////////////////////
std::list<ScreenCap> g_screenCap
{
  {"cl", S_ClrScr,        "\x1b[2J\x1b[H"},
  {"bl", S_Beep,          ""},

  {"ve", S_CursorNormal,  ""},
  {"vi", S_CursorHide,    ""},
  {"vs", S_CursorBold,    ""},

  {"cm", S_GotoXY,        "\x1b[%i%d;%dH"},
  {"ho", S_CursorHome,    ""},
  {"cr", S_CursorBegStr,  "\xd"},
  {"bc", S_CursorLeft,    ""},
  {"le", S_CursorLeft,    "\x8"},
  {"nd", S_CursorRight,   ""},
  {"up", S_CursorUp,      ""},
  {"do", S_CursorDown,    ""},
  {"nl", S_CursorDown,    "\xa"},

  {"AF", S_SetTextColor,  ""},
  {"Sf", S_SetTextColor,  "\x1b[3%dm"},
  {"AB", S_SetFonColor,   ""},
  {"Sb", S_SetFonColor,   "\x1b[4%dm"},
  {"md", S_ColorBold,     ""},
  {"mr", S_Reverse,       ""},
  {"me", S_Normal,        ""},

  {"ic", S_InsertChar,    ""},
  {"IC", S_InsertCharN,   ""},
  {"dc", S_DelChar,       ""},
  {"DC", S_DelCharN,      ""},
  {"cb", S_DelBegOfStr,   ""},
  {"ce", S_DelEndOfStr,   ""},

  {"dl", S_DeleteL,       ""},
  {"DL", S_DeleteLN,      ""},
  {"al", S_InsertL,       ""},
  {"AL", S_InsertLN,      ""},

  {"cs", S_SetScroll,     ""},
  {"im", S_InsertMode,    ""},
  {"ei", S_EInsertMode,   ""},

  {"ac", S_AltCharPairs,  "``aaffggiijjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~"},
  {"as", S_AltCharBegin,  ""},
  {"ae", S_AltCharEnd,    ""},
  {"eA", S_AltCharEnable, ""},

  {"ti", S_TermInit,      ""},
  {"te", S_TermReset,     ""},
  {"op", S_DefaultColor,  "\x1b[0;39;49m"}
};

} //namespace _Console

#endif //WIN32