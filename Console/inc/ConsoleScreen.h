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
#include <vector>
#include <algorithm>
#include <optional>

//////////////////////////////////////////////////////////////////////////////
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
    S_TAB               = 9,  //symbol TAB
    S_LF                = 10, //symbol LF
    ACS_LTEE            = 11,
    ACS_BTEE            = 12,
    S_CR                = 13, //symbol CR
    ACS_PLUS            = 14,
    ACS_SQUARE          = 15,

    ACS_MAX             = 16
};


//////////////////////////////////////////////////////////////////////////////
#define CTEXT_MASK  0x0000ffffl
#define CCOLOR_MASK 0x00ff0000l
#define CATTR_MASK  0xff000000l

#define MAKE_CELL(attr, color, text)    (((static_cast<uint32_t>(color) << 16) & CCOLOR_MASK) | (static_cast<uint32_t>(text) & CTEXT_MASK))
#define GET_CATTR(cell)                 static_cast<uint8_t>(((cell) & CATTR_MASK)  >> 24)
#define GET_CCOLOR(cell)                static_cast<color_t>(((cell) & CCOLOR_MASK) >> 16)
#define GET_CTEXT(cell)                 static_cast<char16_t>((cell) & CTEXT_MASK)

using cell_array = std::vector<cell_t>;

//////////////////////////////////////////////////////////////////////////////
class ScreenBuffer
{
    size_t      m_sizex{};
    size_t      m_sizey{};
    cell_array  m_buffer;

public:
    ScreenBuffer() = default;
    explicit ScreenBuffer(size_t x, size_t y)
    : m_sizex{x}
    , m_sizey{y}
    , m_buffer(x * y)
    {}

    bool SetSize(size_t x = 0, size_t y = 0)
    {
        m_sizex = x;
        m_sizey = y;
        try {
            if (0 != x && 0 != y)
            {
                m_buffer.resize(x * y);
                std::for_each(m_buffer.begin(), m_buffer.end(), [](cell_t& cell) {cell = 0;});
            }
            else
                m_buffer.clear();
        }
        catch (...)
        {
            return false;
        }
        return true;
    }

    void Fill(cell_t fill) { std::for_each(m_buffer.begin(), m_buffer.end(), [fill](cell_t& cell) {cell = fill; }); }
    void GetSize(size_t& x, size_t& y) const { x = m_sizex; y = m_sizey; }
    size_t GetSize() const { return m_sizex * m_sizey; }
    cell_t GetCell(size_t x, size_t y) const
    { 
        if (x >= m_sizex || y >= m_sizey)
        {
            LOG(ERROR) << __FUNC__ << " x=" << x << " y=" << y;
            _assert(!"pos");
            return 0;
        }
        //LOG(DEBUG) << "get x=" << x << " y=" << y << " c=" << std::hex << m_buffer[x + y * m_sizex] << std::dec;
        return m_buffer[x + y * m_sizex]; 
    }
    bool SetCell(size_t x, size_t y, cell_t c)
    {
        //LOG(DEBUG) << "set x=" << x << " y=" << y << " c=" << std::hex << c << std::dec;
        if (x >= m_sizex || y >= m_sizey)
        {
            LOG(ERROR) << __FUNC__ << " x=" << x << " y=" << y;
            _assert(!"pos");
            return false;
        }
        m_buffer[x + y * m_sizex] = c;
        return true;
    }
    bool ScrollBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, pos_t n, scroll_t mode)
    {
        if (left >= m_sizex || right >= m_sizex || top >= m_sizey || bottom >= m_sizey)
        {
            LOG(ERROR) << __FUNC__ << " l=" << left << " t=" << top << " r=" << right << " b=" << bottom;
            _assert(!"pos");
            return false;
        }

        pos_t y;
        switch (mode)
        {
        case scroll_t::SCROLL_UP:
            for (y = top; y <= bottom - n; ++y)
                std::memcpy(m_buffer.data() + left + y * m_sizex, m_buffer.data() + left + (y + n) * m_sizex, (right - left + n) * sizeof(cell_t));
            break;

        case scroll_t::SCROLL_DOWN:
            for (y = bottom; y >= top + n; --y)
                std::memcpy(m_buffer.data() + left + y * m_sizex, m_buffer.data() + left + (y - n) * m_sizex, (right - left + n) * sizeof(cell_t));
            break;

        case scroll_t::SCROLL_LEFT:
            for (y = top; y <= bottom; ++y)
                std::memmove(m_buffer.data() + left + y * m_sizex, m_buffer.data() + left + n + y * m_sizex, (right - left + 1 - n) * sizeof(cell_t));
            break;

        case scroll_t::SCROLL_RIGHT:
            for (y = top; y <= bottom; ++y)
                std::memmove(m_buffer.data() + left + n + y * m_sizex, m_buffer.data() + left + y * m_sizex, (right - left + 1 - n) * sizeof(cell_t));
            break;
        }

        return true;
    }
};

class ConsoleScreen
{
protected:
    cursor_t    m_cursor { cursor_t::CURSOR_HIDE };
    color_t     m_color {};

    pos_t       m_posx {};
    pos_t       m_posy {};
    
    char16_t  m_ACS[ACS_MAX] 
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
    virtual bool Resize() = 0;
    virtual bool SetSize(pos_t sizex, pos_t sizey) = 0;

    virtual bool WriteConsoleTitle(const std::wstring& title) = 0;
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

