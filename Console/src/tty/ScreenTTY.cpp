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

#include "utils/logger.h"
#include "utfcpp/utf8.h"
#include "Console/tty/ScreenTTY.h"
#include "Console/ScreenBuffer.h"

#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <term.h>
#include <sys/ioctl.h>

#include <iomanip>
#include <filesystem>

template<typename T>
inline T IGUR(T func) { return func; }  /* Ignore GCC Unused Result */

using namespace _Utils;

namespace _Console
{

bool ScreenTTY::Init()
{
    if(m_stdout > 0)
        return true;

    //get file handler
    m_stdout = fileno (stdout);

    //load termcap
    for(const auto& cap: g_screenCap)
    {
        ScreenCapType type = cap.cap;
        if(m_cap[type].str.empty())
        {
            char buff[100];
            char* pbuff = buff;

            char* str = tgetstr(const_cast<char*>(cap.id), &pbuff);
            if(nullptr != str)
            {
                //load this cap
                m_cap[type].str = str;
                m_cap[type].id = cap.id;
            }
            else if(*cap.sDefCap)
            {
                //default cap
                m_cap[type].str = cap.sDefCap;
                m_cap[type].id = "--";
            }
        }
    }
    
    GetScreenSize(m_sizex, m_sizey);

    char* term = getenv("TERM");
    if(!strncmp(term, "xterm", 5))
        m_fXTERMconsole = true;
    if(nullptr != strstr(term, "256"))
        m_256colors = true;
    
    LOG(DEBUG) << "term x=" << m_sizex << " y=" << m_sizey
        << " xterm=" << m_fXTERMconsole << " 256=" << m_256colors;

    for(int i = 0; i < CAP_NUMBER; ++i)
        if(!m_cap[i].str.empty())
            LOG(DEBUG) << " Cap[" << i << "][" << m_cap[i].id << "]" << CastEscString(m_cap[i].str);

    //save parameters and use alternative screen buffer
    std::string cmd { "\0337\x1b[?47h"};
    IGUR(write(m_stdout, cmd.c_str(), cmd.size()));

    [[maybe_unused]] bool rc = _WriteStr(m_cap[S_AltCharEnable].str)
        && Flush();

    if(!strncmp(term, "vt100", 5))
    {
        //cut $<2> or $<5>
        std::string str = m_cap[S_GotoXY].str;
        size_t len = str.size();
        if(len > 5 && str[len - 4] == '$' && str[len - 3] == '<')
            m_cap[S_GotoXY].str.resize(len - 4);

        str = m_cap[S_ColorBold].str;
        len = str.size();
        if(len > 5 && str[len - 4] == '$' && str[len - 3] == '<')
            m_cap[S_ColorBold].str.resize(len - 4);

        str = m_cap[S_Normal].str;
        len = str.size();
        if(len > 5 && str[len - 4] == '$' && str[len - 3] == '<')
            m_cap[S_Normal].str.resize(len - 4);
    }

  return true;
}


void ScreenTTY::Deinit()
{
    if(m_stdout <= 0)
        return;

    std::string param = m_cap[S_TermReset].str;
    if(!param.empty())
    {
        IGUR(write(m_stdout, param.c_str(), param.size()));
    }

    //use normal screen buffer and restore parameters
    param = "\x1b[?47l\0338\x1b[0m";
    IGUR(write(m_stdout, param.c_str(), param.size()));

    //set default foreground/background
    param = m_cap[S_DefaultColor].str;
    if(!param.empty())
    {
        IGUR(write(m_stdout, param.c_str(), param.size()));
    }

    m_stdout = -1;
}


//////////////////////////////////////////////////////////////////////////////
bool ScreenTTY::GetScreenSize(pos_t& sizex, pos_t& sizey) const
{
    pos_t x = 0;
    pos_t y = 0;

#ifdef TIOCGWINSZ
    winsize win_struct;

    do
    {
        if ((ioctl(1, TIOCGWINSZ, &win_struct) == 0)
         || (ioctl(0, TIOCGWINSZ, &win_struct) == 0)
         || (ioctl(2, TIOCGWINSZ, &win_struct) == 0))
        {
            x = static_cast<pos_t>(win_struct.ws_col);
            y = static_cast<pos_t>(win_struct.ws_row);
            break;
        }
    }
    while (errno == EINTR);
    LOG(DEBUG) << "screen size (1) x=" << x << " y=" << y;
#endif

    if(x <= 0 || y <= 0)
    {
        char* str = getenv("COLUMNS");
        if(nullptr != str)
            x = atoi(str);

        str = getenv("LINES");
        if(nullptr != str)
            y = atoi(str);

        LOG(DEBUG) << "screen size (2) x=" << x << " y=" << y;
    }

    if(x <= 0 || y <= 0)
    {
        x = tgetnum("co");
        y = tgetnum("li");
        LOG(DEBUG) << "screen size (3) x=" << x << " y=" << y;
    }

    if(x <= 0)
        x = 80;
    if(y <= 0)
        y = 24;

    if(x > MAX_COORD)
        x = MAX_COORD;//max x size
    if(y > MAX_COORD)
        y = MAX_COORD;//max y size

    LOG(DEBUG) << "screen size x=" << x << " y=" << y;
    sizex = x;
    sizey = y;

    return true;
}


bool ScreenTTY::WriteConsoleTitle(const std::string& title)
{
    if(m_stdout <= 0)
        return false;

    //for XTERM
    if(m_fXTERMconsole)
    {
        std::string str {"\x1b]0;" + title + "\7"};
        IGUR(write(m_stdout, str.c_str(), str.size()));
    }

    return true;
}


bool ScreenTTY::GotoXY(pos_t x, pos_t y)
{
    if(m_stdout <= 0)
        return false;
    if(m_cap[S_GotoXY].str.empty())
        return false;

    m_posx = x;
    m_posy = y;

    return _WriteStr(tgoto(m_cap[S_GotoXY].str.c_str(), x, y));
}


bool ScreenTTY::SetCursor(cursor_t cursor)
{
    if(m_cursor == cursor)
        return true;

    std::string_view cap;
    switch(cursor)
    {
    case cursor_t::CURSOR_HIDE:
        cap = m_cap[S_CursorHide].str;
        break;
    case cursor_t::CURSOR_NORMAL:
        cap = m_cap[S_CursorNormal].str;
        break;
    case cursor_t::CURSOR_OVERWRITE:
        if(!m_cap[S_CursorBold].str.empty())
            cap = m_cap[S_CursorBold].str;
        else
            cap = m_cap[S_CursorNormal].str;
        break;
    default:
        return false;
    }
    
    m_cursor = cursor;
    
    int rc = _WriteStr(std::string{cap})
    && Flush();
    
    return rc;
}


bool ScreenTTY::SetTextAttr(color_t color)
{
    if(m_color == color)
        return true;
    if(m_stdout <= 0)
        return false;

    //LOG(DEBUG) << "SetTextAttr " << std::hex << color << std::dec;
    bool rc = true;

    if((color & TEXT_BRIGHT) != (m_color & TEXT_BRIGHT))
    {
        if((color & TEXT_BRIGHT) == 0)
            rc = _WriteStr(m_cap[S_Normal].str);
        else
            rc = _WriteStr(m_cap[S_ColorBold].str);
    }
    
    if(TEXT_COLOR(color) != TEXT_COLOR(m_color) || FON_COLOR(color) != FON_COLOR(m_color))
    {
        if(!m_256colors)
        {
            if(!m_cap[S_SetTextColor].str.empty())
                rc = _WriteStr(tgoto(m_cap[S_SetTextColor].str.c_str(), 0, COLOR_CHANGE(TEXT_COLOR(color))));
            if(!m_cap[S_SetFonColor].str.empty())
                rc = _WriteStr(tgoto(m_cap[S_SetFonColor].str.c_str(), 0, COLOR_CHANGE(FON_COLOR(color))));
        }
        else
        {
            int text = COLOR_CHANGE(TEXT_COLOR(color));
            if(0 != (color & TEXT_BRIGHT))
                text += 8;
            rc = _WriteStr(tgoto("\x1b[38;5;%dm", 0, text));
            rc = _WriteStr(tgoto("\x1b[48;5;%dm", 0, COLOR_CHANGE(FON_COLOR(color))));
        }
    }

    m_color = color;
    return rc;
}


bool ScreenTTY::Flush()
{
    if(m_stdout <= 0)
        return false;
    if(m_OutBuff.empty())
        return true;

    int rc;
    while(-1 == (rc = write(m_stdout, m_OutBuff.c_str(), m_OutBuff.size())))
    {
        if(errno == EINTR)
            continue;

        if(errno == EAGAIN)
        {
            sleep(1);
            continue;
        }

        break;
    }

    //LOG(DEBUG) << "Flush buff size=" << m_OutBuff.size() << "->" << rc;
    //LOG(DEBUG) << CastEscString(m_OutBuff);
    m_OutBuff.clear();

    return rc > 0;
}


bool ScreenTTY::_WriteStr(const std::string& str)
{
    m_OutBuff += str;
    
    if(m_OutBuff.size() >= OUTBUFF_SIZE)
        return Flush();

    return true;
}


bool ScreenTTY::_WriteChar(char c)
{
    m_OutBuff += c;

    if(m_OutBuff.size() >= OUTBUFF_SIZE)
        return Flush();

    return true;
}


bool ScreenTTY::_WriteWChar(char16_t wc)
{
    bool rc;

    if(wc == 0)
        return true;
    else if(wc < ACS_MAX)
    {
        //Alt char set
        std::string str;
        utf8::append(m_ACS[wc], str);
        rc = _WriteStr(str.c_str());
    }
    else 
    {    
        if(wc < 0x80)
        {
            rc = _WriteChar(wc);
        }
        else
        {
            std::string str;
            utf8::append(wc, str);
            rc = _WriteStr(str.c_str());
        }
    }

    if(++m_posx >= m_sizex)
    {
        m_posx = 0;
        if(m_posy < m_sizey - 1)
            ++m_posy;
    }

    return rc;
}


bool ScreenTTY::SetSize(pos_t sizex, pos_t sizey)
{
    m_sizex = sizex;
    m_sizey = sizey;

    return true;
}


bool ScreenTTY::WriteChar(char16_t c)
{
    if(m_stdout <= 0)
        return false;

    return _WriteWChar(c);
}


bool ScreenTTY::WriteStr(const std::u16string& str)
{
    if(m_stdout <= 0)
        return false;

    for(auto c : str)
    {
        bool rc = _WriteWChar(c);
        if(!rc)
            return false;
    }

    return true;
}


bool ScreenTTY::Beep()
{
    bool rc;
    if(!m_cap[S_Beep].str.empty())
        rc = _WriteStr(m_cap[S_Beep].str.c_str());
    else
        rc = _WriteChar(7);
    rc = Flush();
    return rc;
}


bool ScreenTTY::WriteLastChar(char16_t prevC, char16_t lastC)
{
    if(m_stdout <= 0)
        return false;

    //LOG(DEBUG) << "WriteLastChar " << std::hex << prevC << " " << lastC << std::dec;

    bool rc = GotoXY(m_sizex - 2, m_sizey - 1)
    && WriteChar(lastC)
    && GotoXY(m_sizex - 2, m_sizey - 1)
    && _WriteStr(m_cap[S_InsertMode].str);

    if(m_cap[S_InsertMode].str.empty())
        rc = _WriteStr(m_cap[S_InsertChar].str);

    rc = WriteChar(prevC)
    && _WriteStr(m_cap[S_EInsertMode].str);

    return rc;
}

bool ScreenTTY::ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom,
    pos_t n, scroll_t mode, uint32_t* invalidate)
{
    //LOG(DEBUG) << "scroll m=" << static_cast<int>(mode) << " l=" << left << " t=" << top << " r=" << right << " b=" << bottom << " n=" << n;

    bool rc = false;
    std::string cap = m_cap[S_SetScroll].str;

    switch(mode)
    {
    case scroll_t::SCROLL_UP:
        if(bottom - top <= n)
            break;

        if(invalidate)
            *invalidate = INVALIDATE_LEFT | INVALIDATE_RIGHT;

        if(!cap.empty())
            //fix up/down scroll region
            rc = _WriteStr(tgoto(cap.c_str(), bottom, top));

        rc = GotoXY(0, top);
        if(!m_cap[S_DeleteLN].str.empty())
            rc = _WriteStr(tgoto(m_cap[S_DeleteLN].str.c_str(), 0, n));
        else
            for(pos_t i = 0; i < n; ++i)
                rc = _WriteStr(m_cap[S_DeleteL].str.c_str());

        if(!cap.empty())
            rc = _WriteStr(tgoto(cap.c_str(), m_sizey - 1, 0));
        else
        {
            //if fix impossible then insert bottom lines
            rc = GotoXY(0, bottom);
            if(!m_cap[S_InsertLN].str.empty())
                rc = _WriteStr(tgoto(m_cap[S_InsertLN].str.c_str(), 0, n));
            else
                for(pos_t i = 0; i < n; ++i)
                    rc = _WriteStr(m_cap[S_InsertL].str.c_str());
        }
        break;

    case scroll_t::SCROLL_DOWN:
        if(bottom - top <= n)
            break;

        if(invalidate)
            *invalidate = INVALIDATE_LEFT | INVALIDATE_RIGHT;

        if(!cap.empty())
            //fix up/down scroll region
            rc = _WriteStr(tgoto(cap.c_str(), bottom, top));
        else
        {
            //if fix impossible then delete bottom lines before
            rc = GotoXY(0, bottom);
            if(!m_cap[S_DeleteLN].str.empty())
                rc = _WriteStr(tgoto(m_cap[S_DeleteLN].str.c_str(), 0, n));
            else
                for(pos_t i = 0; i < n; ++i)
                    rc = _WriteStr(m_cap[S_DeleteL].str.c_str());
        }

        rc = GotoXY(0, top);
        if(!m_cap[S_InsertLN].str.empty())
            rc = _WriteStr(tgoto(m_cap[S_InsertLN].str.c_str(), 0, n));
        else
            for(pos_t i = 0; i < n; ++i)
                rc = _WriteStr(m_cap[S_InsertL].str.c_str());

        if(!cap.empty())
            rc = _WriteStr(tgoto(cap.c_str(), m_sizey - 1, 0));
        break;

    case scroll_t::SCROLL_LEFT:
    {
        if(right - left <= n)
            break;

        if(invalidate)
            *invalidate = 0;

        rc = _WriteStr(m_cap[S_InsertMode].str.c_str());

        for(pos_t i = top; i <= bottom; ++i)
        {
            rc = GotoXY(left, i);

            if(n > 1 && !m_cap[S_DelCharN].str.empty())
                rc = _WriteStr(tgoto(m_cap[S_DelCharN].str.c_str(), 0, n));
            else
                for(pos_t j = 0; j < n; ++j)
                    rc = _WriteStr(m_cap[S_DelChar].str.c_str());

            //now insert right chars
            rc = GotoXY(right - n + 1, i);

            if(!m_cap[S_InsertMode].str.empty())
            {
                std::string buff(n, ' ');
                rc = _WriteStr(buff);
            }
            else if(!m_cap[S_InsertChar].str.empty())
                for(pos_t j = 0; j < n; ++j)
                    rc = _WriteStr(m_cap[S_InsertChar].str.c_str());
        }

        rc = _WriteStr(m_cap[S_EInsertMode].str.c_str());
        break;
    }

    case scroll_t::SCROLL_RIGHT:
    {
        if(right - left <= n)
            break;

        if(invalidate)
            *invalidate = 0;

        rc = _WriteStr(m_cap[S_InsertMode].str.c_str());

        for(pos_t i = top; i <= bottom; ++i)
        {
            //before delete right chars
            rc = GotoXY(right - n, i);

            if(n > 1 && !m_cap[S_DelCharN].str.empty())
                rc = _WriteStr(tgoto(m_cap[S_DelCharN].str.c_str(), 0, n));
            else
                for(pos_t j = 0; j < n; ++j)
                    rc = _WriteStr(m_cap[S_DelChar].str.c_str());

            rc = GotoXY(left, i);

            if(!m_cap[S_InsertMode].str.empty())
            {
                std::string buff(n, ' ');
                rc = _WriteStr(buff);
            }
            else if(!m_cap[S_InsertChar].str.empty())
                for(pos_t j = 0; j < n; ++j)
                    rc = _WriteStr(m_cap[S_InsertChar].str.c_str());
        }

        rc = _WriteStr(m_cap[S_EInsertMode].str.c_str());
        break;
    }

    default:
        return false;
    }

    return rc;
}

bool ScreenTTY::WriteBlock(
    pos_t left, pos_t top, pos_t right, pos_t bottom,
    const ScreenBuffer& block, pos_t xoffset, pos_t yoffset)
{
    bool rc = false;

    //LOG(DEBUG) << "WriteBlock l=" << left << " t=" << top << " r=" << right << " b=" << bottom;
    
    bool fLast {false};
    if(right == m_sizex - 1 && bottom == m_sizey - 1)
        fLast = 1;

    color_t color {0};
    pos_t sizex = right - left + 1;
    pos_t sizey = bottom - top + 1;
    for(pos_t y = 0; y < sizey; ++y)
    {
        GotoXY(left, top + y);
        for(pos_t x = 0; x < sizex; ++x)
        {
            cell_t c = block.GetCell(xoffset + x, yoffset + y);
            color_t a = GET_CCOLOR(c);
            char16_t ch = GET_CTEXT(c);
            if(color != a)
            {
                //LOG(DEBUG) << "x=" << x << " y=" << y << " a=" << std::hex << a << std::dec;
                rc = SetTextAttr(a);
                color = a;
            }
            
            //LOG(DEBUG) << "x=" << x << " y=" << y << " ch=" << std::hex << ch << std::dec;
            rc = _WriteWChar(ch);
            
            if(fLast && y == sizey - 1 && x == sizex - 2)
                break;
        }
    }

    if(fLast)
    {
        rc = WriteLastChar(
            GET_CTEXT(block.GetCell(m_sizex - 2, m_sizey - 1)), 
            GET_CTEXT(block.GetCell(m_sizex - 1, m_sizey - 1)));
    }

    rc = Flush();
    return rc;
}

} //namespace _Console

#endif //WIN32
