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
#ifndef WIN32

#include "ConsoleScreen.h"
#include "tty/TermcapMap.h"

#include <array>

#define COLOR_CHANGE(color) (((color) & TEXT_BRIGHT) | (((color) & TEXT_RED) >> 2) | ((color) & TEXT_GREEN) | (((color) & TEXT_BLUE) << 2))

enum acs_char : int
{
    ACS_CHLINE          = 'q',
    ACS_CVLINE          = 'x',
    ACS_CURCORNER       = 'k',
    ACS_CULCORNER       = 'l',
    ACS_CLRCORNER       = 'j',
    ACS_CLLCORNER       = 'm',
    ACS_CTTEE           = 'w',
    ACS_CRTEE           = 'u',
    ACS_CLTEE           = 't',
    ACS_CBTEE           = 'v',
    ACS_CPLUS           = 'n',
    ACS_CSQUARE         = '0'
};

class ScreenTTY final : public ConsoleScreen
{
    friend class Console;
    
    inline static const size_t OUTBUFF_SIZE {0x10000};

    const TermcapBuffer&  m_termcap {TermcapBuffer::getInstance()};
    
    int             m_stdout {-1};
    bool            m_fXTERMconsole{false};

    std::string     m_OutBuff;

    struct CapString
    {
        std::string id;
        std::string str;
    };
    std::array<CapString, CAP_NUMBER> m_cap;

public:
    ScreenTTY() = default;
    virtual ~ScreenTTY() override { Deinit(); }

    virtual bool Init() override;
    virtual void Deinit() override;
    virtual bool Resize() override {return true;}
    virtual bool SetSize(pos_t sizex, pos_t sizey) override;

    virtual bool WriteConsoleTitle(const std::wstring& title) override;
    virtual bool Beep() override;
    virtual bool WriteChar(char16_t c) override;
    virtual bool WriteStr(const std::u16string& str) override;

    virtual bool GotoXY(pos_t x, pos_t y) override;
    virtual bool ClrScr() override {return _WriteStr(m_cap[S_ClrScr].str.c_str());}
    virtual bool SetCursor(cursor_t cursor) override;
    virtual bool SetTextAttr(color_t color) override;

    virtual bool Left() override {return _WriteStr(m_cap[S_CursorLeft].str.c_str());}
    virtual bool Right()override {return _WriteStr(m_cap[S_CursorRight].str.c_str());}
    virtual bool Up()   override {return _WriteStr(m_cap[S_CursorUp].str.c_str());}
    virtual bool Down() override {return _WriteStr(m_cap[S_CursorDown].str.c_str());}

    virtual bool ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom,
        pos_t n, scroll_t mode, uint32_t* invalidate = NULL) override;
    virtual bool WriteLastChar(char16_t prevC, char16_t lastC) override;
    virtual bool WriteBlock(
        pos_t left, pos_t top, pos_t right, pos_t bottom,
        const ScreenBuffer& block, pos_t xoffset = 0, pos_t yoffset = 0) override;

    virtual bool Flush() override;

    bool GetScreenSize(pos_t& sizex, pos_t& sizey) const;

private:
    bool Resize(pos_t sizex, pos_t sizey);
    
    bool _WriteChar(char c);
    bool _WriteStr(const std::string& str);
    bool _WriteWChar(char16_t c);
};


#endif //WIN32

