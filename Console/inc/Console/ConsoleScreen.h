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
#pragma once

#include "utils/logger.h"
#include "Types.h"
#include "Color.h"

#include <string>


namespace _Console
{

enum class cursor_t
{
    CURSOR_OFF          = -1,
    CURSOR_HIDE         = 0,
    CURSOR_NORMAL       = 3,
    CURSOR_OVERWRITE    = 5
};

enum invalidateScreen_t : uint32_t
{
    INVALIDATE_NO       = 0,
    INVALIDATE_LEFT     = 1,
    INVALIDATE_RIGHT    = 2
};

enum acs_t
{
    ACS_HLINE           = 1,
    ACS_VLINE           = 2,
    ACS_URCORNER        = 3,
    ACS_ULCORNER        = 4,
    ACS_LRCORNER        = 5,
    ACS_LLCORNER        = 6,
    ACS_TTEE            = 7,
    ACS_RTEE            = 8,
    CS_TAB              = 9,  //symbol TAB
    CS_LF               = 10, //symbol LF
    ACS_LTEE            = 11,
    ACS_BTEE            = 12,
    CS_CR               = 13, //symbol CR
    ACS_PLUS            = 14,
    ACS_SQUARE          = 15,

    ACS_MAX             = 16
};

//////////////////////////////////////////////////////////////////////////////
class ScreenBuffer;

class ConsoleScreen
{
protected:
    cursor_t    m_cursor { cursor_t::CURSOR_HIDE };
    color_t     m_color {};

    pos_t       m_posx {};
    pos_t       m_posy {};
    
    char16_t    m_ACS[ACS_MAX] 
    {
        0x20,   //0
        0x2500, //ACS_HLINE
        0x2502, //ACS_VLINE
        0x2510, //ACS_URCORNER
        0x250c, //ACS_ULCORNER
        0x2518, //ACS_LRCORNER
        0x2514, //ACS_LLCORNER
        0x252c, //ACS_TTEE
        0x2524, //ACS_RTEE
        0x20,   //S_TAB
        0x20,   //S_LF
        0x251c, //ACS_LTEE
        0x2534, //ACS_BTEE
        0x20,   //S_CR
        0x253c, //ACS_PLUS
        0x2588  //ACS_SQUARE
    };

public:
    pos_t       m_sizex {0};
    pos_t       m_sizey {0};

    virtual ~ConsoleScreen() = default;

    virtual bool Init() = 0;
    virtual void Deinit() = 0;
    virtual bool SetSize(pos_t sizex, pos_t sizey) = 0;

    virtual bool WriteConsoleTitle(const std::string& title) = 0;
    virtual bool Beep() = 0;
    virtual bool WriteChar(char16_t c) = 0;
    virtual bool WriteStr(const std::u16string& str) = 0;

    virtual bool GotoXY(pos_t x, pos_t y) = 0;
    virtual bool ClrScr() = 0;
    virtual bool SetCursor(cursor_t cursor) = 0;
    virtual bool SetTextAttr(color_t color) = 0;

    virtual bool Left() = 0;
    virtual bool Right() = 0;
    virtual bool Up() = 0;
    virtual bool Down() = 0;

    virtual bool ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom,
        pos_t n, scroll_t mode, uint32_t* invalidate = NULL) = 0;
    virtual bool WriteLastChar(char16_t prevC, char16_t lastC) = 0;
    virtual bool WriteBlock(
        pos_t left, pos_t top, pos_t right, pos_t bottom,
        const ScreenBuffer& block, pos_t xoffset = 0, pos_t yoffset = 0) = 0;

    virtual bool Flush() = 0;
};

} //namespace _Console
