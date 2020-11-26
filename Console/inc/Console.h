#pragma once
#include "win32/InputWin32.h"
#include "win32/ScreenWin32.h"
#include "tty/InputTTY.h"
#include "tty/ScreenTTY.h"


//////////////////////////////////////////////////////////////////////////////
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
    
    bool InputPending(const std::chrono::milliseconds& waitTime = 500ms)
        {return m_input.InputPending(waitTime);}
    bool SwitchToStdConsole()
        {return m_input.SwitchToStdConsole();}
    bool RestoreConsole()
        {return m_input.RestoreConsole();}
    bool PutInput(const input_t code)
        {return m_input.PutInput(code);}
    size_t GetInputLen()
        {return m_input.GetInputLen();}
    input_t GetInput()
        {return m_input.GetInput();}

    bool WriteConsoleTitle(const std::wstring& title)
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
