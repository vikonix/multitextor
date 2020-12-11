/*
FreeBSD License

Copyright (c) 2020 vikonix: valeriy.kovalev.software@gmail.com
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
#include "KeyCodes.h"
#include "Types.h"
//#include "termdb/termdb.hpp"


//////////////////////////////////////////////////////////////////////////////
enum ScreenCapType
{
    S_ClrScr = 0,
    S_Beep,

    S_CursorNormal,
    S_CursorHide,
    S_CursorBold,

    S_GotoXY,
    S_CursorHome,
    S_CursorBegStr,
    S_CursorLeft,
    S_CursorRight,
    S_CursorUp,
    S_CursorDown,

    S_SetTextColor,
    S_SetFonColor,
    S_ColorBold,
    S_Reverse,
    S_Normal,

    S_InsertChar,
    S_InsertCharN,
    S_DelChar,
    S_DelCharN,
    S_DelBegOfStr,
    S_DelEndOfStr,

    S_DeleteL,
    S_DeleteLN,
    S_InsertL,
    S_InsertLN,

    S_SetScroll,
    S_InsertMode,
    S_EInsertMode,

    S_AltCharPairs,
    S_AltCharBegin,
    S_AltCharEnd,
    S_AltCharEnable,

    S_TermInit,
    S_TermReset,
    S_Reset1str,

    S_TestCap1,
    S_TestCap2,
    S_TestCap3,
    S_TestCap4,

    CAP_NUMBER //41
};

//////////////////////////////////////////////////////////////////////////////
struct KeyCap
{
    const char*     id;
    input_t         code;
};

struct ScreenCap 
{
    const char*     id;
    ScreenCapType   cap;
    const char*     sDefCap;
};

//////////////////////////////////////////////////////////////////////////////
extern KeyCap    g_keyCap[];
extern ScreenCap g_screenCap[];

#ifndef WIN32

#include <term.h>
#include <cstdlib>

class TermcapBuffer
{
public:
    static TermcapBuffer& getInstance()
    {
        static TermcapBuffer instance;
        return instance;
    }

    void LoadTermcap() const
    {
        static char termcapBuff[0x2000];
        char* term;
        if (NULL == (term = getenv("TERM")))
            return;

        if (1 != tgetent(termcapBuff, term))
            return;
    }

    TermcapBuffer(const TermcapBuffer&) = delete;
    void operator= (const TermcapBuffer&) = delete;

private:
    TermcapBuffer() {LoadTermcap();}
};
#endif //WIN32


