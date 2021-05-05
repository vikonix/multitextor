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

#include <vector>
#include <algorithm>


namespace _Console
{

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
    bool SetColor(size_t x, size_t y, color_t c)
    {
        //LOG(DEBUG) << "set color x=" << x << " y=" << y << " c=" << std::hex << c << std::dec;
        if (x >= m_sizex || y >= m_sizey)
        {
            LOG(ERROR) << __FUNC__ << " x=" << x << " y=" << y;
            _assert(!"pos");
            return false;
        }
        m_buffer[x + y * m_sizex] = MAKE_CELL(0, c, m_buffer[x + y * m_sizex]);
        return true;
    }
    
    bool ScrollBlock(size_t left, size_t top, size_t right, size_t bottom, size_t n, scroll_t mode)
    {
        if (left >= m_sizex || right >= m_sizex || top >= m_sizey || bottom >= m_sizey)
        {
            LOG(ERROR) << __FUNC__ << " l=" << left << " t=" << top << " r=" << right << " b=" << bottom;
            _assert(!"pos");
            return false;
        }

        size_t y;
        cell_t* buff = m_buffer.data();
        switch (mode)
        {
        case scroll_t::SCROLL_UP:
            if (bottom < n)
                bottom = n;
            for (y = top; y <= bottom - n; ++y)
                std::memcpy(buff + left + y * m_sizex, buff + left + (y + n) * m_sizex, (right - left + 1) * sizeof(cell_t));
            break;

        case scroll_t::SCROLL_DOWN:
            for (y = bottom; y >= top + n && y <= bottom; --y)
                std::memcpy(buff + left + y * m_sizex, buff + left + (y - n) * m_sizex, (right - left + 1) * sizeof(cell_t));
            break;

        case scroll_t::SCROLL_LEFT:
            for (y = top; y <= bottom; ++y)
                std::memmove(buff + left + y * m_sizex, buff + left + n + y * m_sizex, (right - left + 1 - n) * sizeof(cell_t));
            break;

        case scroll_t::SCROLL_RIGHT:
            for (y = top; y <= bottom; ++y)
                std::memmove(buff + left + n + y * m_sizex, buff + left + y * m_sizex, (right - left + 1 - n) * sizeof(cell_t));
            break;
        }

        return true;
    }
};

} //namespace _Console
