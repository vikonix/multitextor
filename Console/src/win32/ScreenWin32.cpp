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
#ifdef WIN32

#include "utils/logger.h"
#include "utfcpp/utf8.h"
#include "Console/win32/ScreenWin32.h"
#include "Console/ScreenBuffer.h"


//////////////////////////////////////////////////////////////////////////////
namespace _Console
{

//vintage color palette
COLORREF ScreenWin32::s_colorPalette[16]
{
    //0x00bbggrr
    0x000000,
    0x800000,
    0x008000,
    0x808000,
    0x000080,
    0x800080,
    0x008080,
    0xc0c0c0,
    0x808080,
    0xff0000,
    0x00ff00,
    0xffff00,
    0x0000ff,
    0xff00ff,
    0x00ffff,
    0xffffff
};

bool ScreenWin32::Init()
{
    if (INVALID_HANDLE_VALUE != m_hStdout)
        return true;

    // stdout may have been redirected.
    m_hStdout = CreateFile (L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == m_hStdout)
    {
        LOG(ERROR) << __FUNC__ << "ERROR get Handle err=" << GetLastError();
        return false;
    }

    m_saveInfoEx.cbSize = sizeof(m_saveInfoEx);
    bool rc = GetConsoleScreenBufferInfoEx(m_hStdout, &m_saveInfoEx);
    if (!rc)
    {
        LOG(ERROR) << __FUNC__ << "ERROR GetConsoleScreenBufferInfoEx err=" << GetLastError();
        m_saveInfoEx.cbSize = 0;
    }
    else
    {
        CONSOLE_SCREEN_BUFFER_INFOEX info{ m_saveInfoEx };
        std::memcpy(info.ColorTable, s_colorPalette, sizeof(m_saveInfoEx.ColorTable));
        rc = SetConsoleScreenBufferInfoEx(m_hStdout, &info);
        if (!rc)
            LOG(ERROR) << __FUNC__ << "ERROR SetConsoleScreenBufferInfoEx err=" << GetLastError();
    }

    rc = GetConsoleScreenBufferInfo(m_hStdout, &m_saveInfo);
    if (!rc)
        LOG(ERROR) << __FUNC__ << "ERROR GetConsoleScreenBufferInfo err=" << GetLastError();

    GetConsoleCursorInfo(m_hStdout, &m_saveCursor);
    LOG(DEBUG) << __FUNC__ << " x=" << m_saveInfo.dwSize.X << " y=" << m_saveInfo.dwSize.Y;
    LOG(INFO) << "cursor size=" << m_saveCursor.dwSize << "%";

//    SetWindowPos(GetConsoleWindow(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);

    return SetSize(m_saveInfo.dwMaximumWindowSize.X, m_saveInfo.dwMaximumWindowSize.Y);
}

bool ScreenWin32::SetSize(pos_t sizex, pos_t sizey)
{
    if (sizex == 0 || sizey == 0)
        return false;

    LOG(DEBUG) << __FUNC__ << " x=" << sizex << " y=" << sizey;

    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    bool rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);
    if(!rc)
        LOG(ERROR) << __FUNC__ << "ERROR GetConsoleScreenBufferInfo err=" << GetLastError();

    LOG(INFO) << "Cur size=" << sbInfo.dwSize.X << "/" << sbInfo.dwSize.Y
        << " pos=" << sbInfo.srWindow.Left << "/" << sbInfo.srWindow.Top << "/" << sbInfo.srWindow.Right << "/" << sbInfo.srWindow.Bottom
        << " max=" << sbInfo.dwMaximumWindowSize.X << "/" << sbInfo.dwMaximumWindowSize.Y;

    if (sizex > sbInfo.dwMaximumWindowSize.X || sizey > sbInfo.dwMaximumWindowSize.Y)
    {
        COORD sizeM{ MAX_COORD, MAX_COORD };
        rc = SetConsoleScreenBufferSize(m_hStdout, sizeM);
        if (!rc)
            LOG(ERROR) << __FUNC__ << "ERROR SetConsoleScreenBufferSizeM err=" << GetLastError();

        rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);
        if (!rc)
            LOG(ERROR) << __FUNC__ << "ERROR GetConsoleScreenBufferInfoM err=" << GetLastError();

        LOG(DEBUG) << "max=" << sbInfo.dwMaximumWindowSize.X << "/" << sbInfo.dwMaximumWindowSize.Y;
    }

    if(sizex > sbInfo.dwMaximumWindowSize.X)
        sizex = sbInfo.dwMaximumWindowSize.X;

    if(sizey > sbInfo.dwMaximumWindowSize.Y)
        sizey = sbInfo.dwMaximumWindowSize.Y;

    SMALL_RECT rect {0, 0, sizex - 1, sizey - 1};
    rc = SetConsoleWindowInfo(m_hStdout, TRUE, &rect);
    if(!rc)
        LOG(ERROR) << __FUNC__ << "ERROR SetConsoleWindowInfoM1 err=" << GetLastError();

    COORD size { sizex, sizey };
    rc = SetConsoleScreenBufferSize(m_hStdout, size);
    if(!rc)
        LOG(ERROR) << __FUNC__ << "ERROR SetConsoleScreenBufferSizeM1 err=" << GetLastError();

    rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);
    if (!rc)
        LOG(ERROR) << __FUNC__ << "ERROR GetConsoleScreenBufferInfo err=" << GetLastError();

    m_sizex = sbInfo.dwSize.X;
    m_sizey = sbInfo.dwSize.Y;
    LOG(INFO) << "Screen size=" << m_sizex << "/" << m_sizey;

    return rc;
}

void ScreenWin32::Deinit()
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return;

    //SetConsoleTextAttribute(m_hStdout, m_saveInfo.wAttributes);
    ClrScr();

    [[maybe_unused]] bool rc;
    if (m_saveInfoEx.cbSize != 0)
    {
        rc = SetConsoleScreenBufferInfoEx(m_hStdout, &m_saveInfoEx);
    }
    else
    {
        rc = SetConsoleScreenBufferSize(m_hStdout, m_saveInfo.dwSize);
    }
    LOG(DEBUG) << __FUNC__ << "Restore x=" << m_saveInfo.dwSize.X << " y=" << m_saveInfo.dwSize.Y;

    CloseHandle(m_hStdout);
    m_hStdout = INVALID_HANDLE_VALUE;
}

bool ScreenWin32::WriteConsoleTitle(const std::string& title)
{
    //LOG(DEBUG) << "WriteConsoleTitle '" << title << "'";
    auto str = utf8::utf8to16(title);
    bool rc = SetConsoleTitle(reinterpret_cast<LPCTSTR>(str.c_str()));
    return rc;
}

bool ScreenWin32::Beep()
{
    bool rc = ::Beep(1200, 100);
    return rc;
}

bool ScreenWin32::WriteChar(char16_t wc)
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;
    
    wchar_t c = (wc < ACS_MAX) ? m_ACS[wc] : wc;

    DWORD written;
    bool rc = WriteConsole(m_hStdout, &c, 1, &written, NULL);

    return rc;
}

bool ScreenWin32::WriteStr(const std::u16string& str)
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    std::u16string outstr;
    for (auto ch : str)
    {
        if (ch < ACS_MAX)
            ch = m_ACS[ch];
        outstr += ch;
    }
  
    DWORD written;
    bool rc = WriteConsole(m_hStdout, outstr.c_str(), static_cast<DWORD>(outstr.size()), &written, NULL);

    return rc;
}

bool ScreenWin32::GotoXY(pos_t x, pos_t y)
{
    //LOG(DEBUG) << "GotoXY x=" << x << " y=" << y; 

    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    COORD CurPos{x, y};
    bool rc = SetConsoleCursorPosition(m_hStdout, CurPos);
    return rc;
}

bool ScreenWin32::ClrScr()
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    DWORD written;
    COORD coord {0, 0};

    bool rc = FillConsoleOutputAttribute(
        m_hStdout, m_color, m_sizex * m_sizey, coord, &written);

    rc = FillConsoleOutputCharacter(
        m_hStdout, ' ', m_sizex * m_sizey, coord, &written);

    rc = GotoXY(0, 0);

    return rc;
}

bool ScreenWin32::SetCursor(cursor_t cursor)
{
    //LOG(DEBUG) << "SetCursor " << static_cast<uint32_t>(cursor);
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    CONSOLE_CURSOR_INFO cInfo;
    switch(cursor)
    {
    case cursor_t::CURSOR_HIDE:
        cInfo.bVisible = 0;
        cInfo.dwSize   = 10;
        break;
    case cursor_t::CURSOR_NORMAL:
        cInfo.bVisible = 1;
        cInfo.dwSize   = 15;
        break;
    case cursor_t::CURSOR_OVERWRITE:
        cInfo.bVisible = 1;
        cInfo.dwSize   = 99;
        break;
    default:
        return false;
    }
    m_cursor = cursor;

    return SetConsoleCursorInfo(m_hStdout, &cInfo);
}

bool ScreenWin32::SetTextAttr(color_t color)
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    if(m_color == color)
        return true;

    //LOG(DEBUG) << "SetTextAttr " << color;
    m_color = color;
    return SetConsoleTextAttribute(m_hStdout, color);
}

bool ScreenWin32::Left()
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    bool rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);

    if (rc && sbInfo.dwCursorPosition.X > 0)
        rc = GotoXY(--sbInfo.dwCursorPosition.X, sbInfo.dwCursorPosition.Y);

    return rc;
}

bool ScreenWin32::Right()
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    bool rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);

    if (rc && sbInfo.dwCursorPosition.X < m_sizex - 1)
        rc = GotoXY(++sbInfo.dwCursorPosition.X, sbInfo.dwCursorPosition.Y);

    return rc;
}

bool ScreenWin32::Up()
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    bool rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);

    if (rc && sbInfo.dwCursorPosition.Y > 0)
        rc = GotoXY(sbInfo.dwCursorPosition.X, --sbInfo.dwCursorPosition.Y);

    return rc;
}

bool ScreenWin32::Down()
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    bool rc = GetConsoleScreenBufferInfo(m_hStdout, &sbInfo);

    if (rc && sbInfo.dwCursorPosition.Y < m_sizey - 1)
        rc = GotoXY(sbInfo.dwCursorPosition.X, ++sbInfo.dwCursorPosition.Y);

    return rc;
}

bool ScreenWin32::ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom,
    pos_t n, scroll_t mode, uint32_t* invalidate)
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    SMALL_RECT scrollRect { left, top, right, bottom };
    SMALL_RECT clipRect { scrollRect };

    COORD coordDest { left, top };
    switch(mode)
    {
    case scroll_t::SCROLL_UP:
        scrollRect.Top += n;
        break;

    case scroll_t::SCROLL_DOWN:
        scrollRect.Bottom -= n;
        coordDest.Y += n;
        break;

    case scroll_t::SCROLL_LEFT:
        scrollRect.Left += n;
        break;

    case scroll_t::SCROLL_RIGHT:
        scrollRect.Right -= n;
        coordDest.X += n;
        break;

    default:
        return false;
    }

    //Fill
    CHAR_INFO fill;
    fill.Attributes       = m_color;
    fill.Char.UnicodeChar = ' ';

    //Scroll
    bool rc = ScrollConsoleScreenBuffer(
        m_hStdout,      //screen buffer handle
        &scrollRect,    //scrolling rectangle
        &clipRect,      //clipping rectangle
        coordDest,      //top left destination cell
        &fill);         //fill character and color

    if(invalidate)
        *invalidate = INVALIDATE_NO;

  return rc;
}

bool ScreenWin32::WriteLastChar(char16_t prevC, char16_t lastC)
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    bool rc =
        GotoXY(m_sizex - 2, m_sizey - 1) &&
        WriteChar(lastC) &&
        ScrollBlock(m_sizex - 2, m_sizey - 1, m_sizex - 1, m_sizey - 1, 1, scroll_t::SCROLL_RIGHT) &&

        GotoXY(m_sizex - 2, m_sizey - 1) &&
        WriteChar(prevC);

    return rc;
}

bool ScreenWin32::WriteBlock(
    pos_t left, pos_t top, pos_t right, pos_t bottom,
    const ScreenBuffer& block, pos_t xoffset, pos_t yoffset)
{
    if (INVALID_HANDLE_VALUE == m_hStdout)
        return false;

    //LOG(DEBUG) << "WriteBlock l=" << left << " t=" << top << " r=" << right << " b=" << bottom << " xoffset=" << xoffset << " yoffset=" << yoffset;
    //_assert(left == xoffset && top == yoffset);

    size_t sizex = static_cast<size_t>(right - left + 1);
    size_t sizey = static_cast<size_t>(bottom - top + 1);

    cell_array outBuff(sizex * sizey);

    for (size_t y = 0; y < sizey; ++y)
        for (size_t x = 0; x < sizex; ++x)
        {
            cell_t c = block.GetCell(xoffset + x,  yoffset + y);
            if (GET_CTEXT(c) < ACS_MAX)
            {
                c = m_ACS[GET_CTEXT(c)] | (c & CCOLOR_MASK);
            }
            outBuff[x + y * sizex] = c & (CTEXT_MASK | CCOLOR_MASK);
        }

    COORD cBuffSize;          // size of data buffer
    COORD cBuffCoord;         // cell coordinates
    SMALL_RECT srWriteRegion; // rectangle to write

    cBuffSize.X = static_cast<pos_t>(sizex);
    cBuffSize.Y = static_cast<pos_t>(sizey);
    cBuffCoord.X = 0;
    cBuffCoord.Y = 0;
    srWriteRegion.Left = left;
    srWriteRegion.Top = top;
    srWriteRegion.Right = right;
    srWriteRegion.Bottom = bottom;

    bool rc = WriteConsoleOutput(
        m_hStdout,                  // handle to screen buffer
        (CHAR_INFO*)outBuff.data(),  // data buffer
        cBuffSize,                  // size of data buffer
        cBuffCoord,                 // cell coordinates
        &srWriteRegion              // rectangle to write
    );
    if (!rc)
        LOG(ERROR) << __FUNC__ << "WriteBlock err=" << GetLastError();

    return rc;
}

} //namespace _Console

#endif //WIN32
