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
#ifdef WIN32

#include "Console/ConsoleScreen.h"

#include "windows.h"

class ScreenWin32 final : public ConsoleScreen
{
    HANDLE  m_hStdout { INVALID_HANDLE_VALUE };
    pos_t   m_scrSizeX {0};
    pos_t   m_scrSizeY {0};

    pos_t   m_savex {0};
    pos_t   m_savey {0};

public:
    ScreenWin32() = default;
    virtual ~ScreenWin32() override { Deinit(); }

    virtual bool Init() override;
    virtual void Deinit() override;
    virtual bool Resize() override;
    virtual bool SetSize(pos_t sizex, pos_t sizey) override;
    
    virtual bool WriteConsoleTitle(const std::string& title) override;
    virtual bool Beep() override;
    virtual bool WriteChar(char16_t c) override;
    virtual bool WriteStr(const std::u16string& str) override;

    virtual bool GotoXY(pos_t x, pos_t y) override;
    virtual bool ClrScr() override;
    virtual bool SetCursor(cursor_t cursor) override;
    virtual bool SetTextAttr(color_t color) override;


    virtual bool Left() override;
    virtual bool Right() override;
    virtual bool Up() override;
    virtual bool Down() override;

    virtual bool ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom,
        pos_t n, scroll_t mode, uint32_t* invalidate = NULL) override;
    virtual bool WriteLastChar(char16_t prevC, char16_t lastC) override;
    virtual bool WriteBlock(
        pos_t left, pos_t top, pos_t right, pos_t bottom,
        const ScreenBuffer& block, pos_t xoffset = 0, pos_t yoffset = 0) override;

    virtual bool Flush() override { return true; };
};

#endif //WIN32

