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
#include "win32/InputWin32.h"
#include "win32/ScreenWin32.h"
#include "tty/InputTTY.h"
#include "tty/ScreenTTY.h"


//////////////////////////////////////////////////////////////////////////////
namespace _Console
{

using keybuff_t = std::list<input_t>;


class Console final
{
#ifdef WIN32
    InputWin32  m_input;
    ScreenWin32 m_screen;
#else
    InputTTY    m_input;
    ScreenTTY   m_screen;
#endif

public:
    bool Init()
    {
#ifndef WIN32        
        m_input.m_ResizeCallback = [this](pos_t& x, pos_t& y) {
            bool rc = m_screen.GetScreenSize(x, y)
                   && m_screen.SetSize(x, y);
            return rc;       
        };
#endif        
        return m_input.Init() 
            && m_screen.Init();
    }
    
    void Deinit()
        {m_input.Deinit(); m_screen.Deinit();}

    bool SetScreenSize([[maybe_unused]]pos_t sizex, [[maybe_unused]] pos_t sizey)
    { 
#ifdef WIN32        
        return m_screen.SetSize(sizex, sizey); 
#else
        return true;
#endif
    }

    bool InputPending(const std::chrono::milliseconds& waitTime = 500ms)
        {return m_input.InputPending(waitTime);}
    bool PutInput(const input_t code)
        {return m_input.PutInput(code);}
    bool PutMacro(const input_t code)
        {return m_input.PutMacro(code);}
    size_t GetInputLen()
        {return m_input.GetInputLen();}
    input_t GetInput()
        {return m_input.GetInput();}
    void ClearMacro()
        {m_input.ClearMacro();}
    bool PlayMacro()
        {return m_input.PlayMacro();}

    bool WriteConsoleTitle(const std::string& title)
        {return m_screen.WriteConsoleTitle(title);}
    bool Beep()
        {return m_screen.Beep();}
    bool WriteChar(char16_t c)
        {return m_screen.WriteChar(c);}
    bool WriteStr(const std::u16string& str)
        {return m_screen.WriteStr(str);}

    bool GotoXY(pos_t x, pos_t y)
        {return m_screen.GotoXY(x, y);}
    bool ClrScr()
        {return m_screen.ClrScr();}
    bool SetCursor(cursor_t cursor)
        {return m_screen.SetCursor(cursor);}
    bool SetTextAttr(color_t color)
        {return m_screen.SetTextAttr(color);}

    bool Left()
        {return m_screen.Left();}
    bool Right()
        {return m_screen.Left();}
    bool Up()
        {return m_screen.Up();}
    bool Down()
        {return m_screen.Down();}

    bool ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom,
        pos_t n, scroll_t mode, uint32_t* invalidate = NULL)
        {return m_screen.ScrollBlock(left, top, right, bottom, n, mode, invalidate);}
    bool WriteLastChar(char16_t prevC, char16_t lastC)
        {return m_screen.WriteLastChar(prevC, lastC);}
    bool WriteBlock(
        pos_t left, pos_t top, pos_t right, pos_t bottom,
        const ScreenBuffer& block, pos_t xoffset = 0, pos_t yoffset = 0)
        {return m_screen.WriteBlock(left, top, right, bottom, block, xoffset, yoffset);}

    bool Flush()
        {return m_screen.Flush();}

    void GetScreenSize(pos_t& sizex, pos_t& sizey) const
    {
        sizex = m_screen.m_sizex;
        sizey = m_screen.m_sizey;
    }
};

} //namespace _Console
