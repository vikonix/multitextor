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

#include "utils/SymbolType.h"
#include "EditorWnd.h"
#include "EditorCmd.h"
#include "WndManager.h"
#include "EditorApp.h"

#define USE_SCROLL

#ifndef WIN32
    #define ONLY_SCREEN_SCROLL
#endif


bool EditorWnd::MovePos(input_t cmd)
{
    pos_t x = K_GET_X(cmd);
    pos_t y = K_GET_Y(cmd);

    if (x < m_sizeX && y < m_sizeY)
    {
        m_cursorx = x;
        m_cursory = y;
    }
    
    return true;
}

bool EditorWnd::MoveLeft(input_t cmd)
{
    pos_t step = K_GET_CODE(cmd);
    size_t offset = m_xOffset;

    if (!step)
    {
        if (m_cursorx)
            --m_cursorx;
        else if (offset)
            --offset;
    }
    else
    {
        if (m_cursorx >= step)
            m_cursorx -= static_cast<pos_t>(step);
        else
        {
            step -= m_cursorx;
            m_cursorx = 0;
            if (offset > static_cast<size_t>(step))
                offset -= step;
            else
                offset = 0;
        }
    }

    if (offset != m_xOffset)
    {
        size_t dx = m_xOffset - offset;
        m_xOffset = offset;

#ifdef USE_SCROLL
        if (dx <= 8
#ifdef ONLY_SCREEN_SCROLL
            && m_sizeX > WndManager::getInstance().m_sizex - 8
#endif
            )
        {
            Scroll(static_cast<pos_t>(dx), scroll_t::SCROLL_RIGHT);
            InvalidateRect(0, 0, static_cast<pos_t>(dx), m_sizeY);
        }
        else
#endif
            InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveRight(input_t cmd)
{
    pos_t step = K_GET_CODE(cmd);
    size_t offset = m_xOffset;

    if (!step)
    {
        if (m_cursorx < m_sizeX - 1)
            ++m_cursorx;
        else if (offset < static_cast<size_t>(MAX_STRLEN - m_sizeX))
            ++offset;
    }
    else
    {
        if (m_cursorx < m_sizeX - step)
            m_cursorx += static_cast<pos_t>(step);
        else
        {
            step -= m_sizeX - 1 - m_cursorx;
            m_cursorx = m_sizeX - 1;

            if (offset < static_cast<size_t>(MAX_STRLEN - m_sizeX - step))
                offset += step;
            else
                offset = MAX_STRLEN - m_sizeX;
        }
    }

    if (offset != m_xOffset)
    {
        size_t dx = offset - m_xOffset;
        m_xOffset = offset;

#ifdef USE_SCROLL
        if (dx <= 8
#ifdef ONLY_SCREEN_SCROLL
            && m_sizeX > WndManager::getInstance().m_sizex - 8
#endif
            )
        {
            Scroll(static_cast<pos_t>(dx), scroll_t::SCROLL_LEFT);
            InvalidateRect(static_cast<pos_t>(m_sizeX - dx), 0, static_cast<pos_t>(dx), m_sizeY);
        }
        else
#endif
            InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveUp(input_t cmd)
{
    pos_t step = K_GET_CODE(cmd);
    size_t line = m_firstLine;

    if (!step)
    {
        if (m_cursory)
            --m_cursory;
        else if (line)
            --line;
    }
    else
    {
        if (line > static_cast<size_t>(step))
            line -= step;
        else if (line)
            line = 0;
        else if (m_cursory > step)
            m_cursory -= static_cast<pos_t>(step);
        else
            m_cursory = 0;
    }

    if (m_firstLine != line)
    {
        size_t dy = m_firstLine - line;
        m_firstLine = line;

#ifdef USE_SCROLL
        if (dy <= 8
#ifdef ONLY_SCREEN_SCROLL
            && m_sizeX > WndManager::getInstance().m_sizex - 8
#endif
            )
        {
            Scroll(static_cast<pos_t>(dy), scroll_t::SCROLL_DOWN);
            InvalidateRect(0, 0, m_sizeX, static_cast<pos_t>(dy));
        }
        else
#endif
            InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }
    return true;
}

bool EditorWnd::MoveDown(input_t cmd)
{
    pos_t step = K_GET_CODE(cmd);
    size_t line = m_firstLine;

    if (!step)
    {
        if (m_cursory < m_sizeY - 1)
            ++m_cursory;
        else
            ++line;
    }
    else
    {
        line += step;
    }

    if (m_firstLine != line)
    {
        size_t numLine = m_editor->GetStrCount();
        if (line < numLine - m_sizeY / 4)
        {
            size_t dy = line - m_firstLine;
            m_firstLine = line;

#ifdef USE_SCROLL
            if (dy <= 8
#ifdef ONLY_SCREEN_SCROLL
                && m_sizeX > WndManager::getInstance().m_sizex - 8
#endif
                )
            {
                Scroll(static_cast<pos_t>(dy), scroll_t::SCROLL_UP);
                InvalidateRect(0, static_cast<pos_t>(m_sizeY - dy), m_sizeX, static_cast<pos_t>(dy));
            }
            else
#endif
                InvalidateRect(0, 0, m_sizeX, m_sizeY);
        }
        else if (m_cursory < m_sizeY - 1)
        {
            if (m_cursory + step < m_sizeY - 1)
                m_cursory += static_cast<pos_t>(step);
            else
                m_cursory = m_sizeY - 1;
        }
    }
    
    return true;
}

bool EditorWnd::MoveScrollLeft(input_t cmd)
{
    size_t step = K_GET_CODE(cmd);
    if (!m_xOffset)
        return MoveLeft(cmd);

    int offset = static_cast<int>(m_xOffset) - static_cast<int>((step ? step : 1));
    if (offset < 0)
        offset = 0;

    if (m_xOffset != static_cast<size_t>(offset))
    {
        size_t dx = m_xOffset - offset;
        m_xOffset = offset;

#ifdef USE_SCROLL
        if (dx <= 8
#ifdef ONLY_SCREEN_SCROLL
            && m_sizeX > WndManager::getInstance().m_sizex - 8
#endif
            )
        {
            Scroll(static_cast<pos_t>(dx), scroll_t::SCROLL_RIGHT);
            InvalidateRect(0, 0, static_cast<pos_t>(dx), m_sizeY);
        }
        else
#endif
            InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveScrollRight(input_t cmd)
{
    size_t step = K_GET_CODE(cmd);

    if (m_xOffset >= static_cast<size_t>(MAX_STRLEN - m_sizeX))
        return MoveRight(cmd);

    size_t offset = m_xOffset + (step ? step : 1);
    if (offset > static_cast<size_t>(MAX_STRLEN - m_sizeX))
        offset = MAX_STRLEN - m_sizeX;

    if (offset != m_xOffset)
    {
        size_t dx = offset - m_xOffset;
        m_xOffset = offset;

#ifdef USE_SCROLL
        if (dx <= 8
#ifdef ONLY_SCREEN_SCROLL
            && m_sizeX > WndManager::getInstance().m_sizex - 8
#endif
            )
        {
            Scroll(static_cast<pos_t>(dx), scroll_t::SCROLL_LEFT);
            InvalidateRect(static_cast<pos_t>(m_sizeX - dx), 0, static_cast<pos_t>(dx), m_sizeY);
        }
        else
#endif
            InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MovePageUp([[maybe_unused]]input_t cmd)
{
    size_t line = m_firstLine;

    if (line >= static_cast<size_t>(m_sizeY - 1))
        line -= m_sizeY - 1;
    else
    {
        line = 0;
        m_cursory = 0;
    }

    if (m_firstLine != line)
    {
        m_firstLine = line;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MovePageDown([[maybe_unused]]input_t cmd)
{
    size_t numLine = m_editor->GetStrCount();
    if (m_firstLine + m_sizeY - 1 < numLine)
    {
        m_firstLine += m_sizeY - 1;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }
    else
    {
        MoveDown(m_sizeY - 1);
    }

    return true;
}

bool EditorWnd::MoveFileBegin([[maybe_unused]]input_t cmd)
{
    m_cursorx = 0;
    m_cursory = 0;

    if (m_xOffset != 0 || m_firstLine != 0)
    {
        m_xOffset = 0;
        m_firstLine = 0;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveFileEnd([[maybe_unused]]input_t cmd)
{
    size_t numLine = m_editor->GetStrCount();

    size_t x = 0;
    size_t line = 0;
    m_cursorx = 0;
    if (numLine >= static_cast<size_t>(m_sizeY))
    {
        line = numLine - m_sizeY + 1;
        m_cursory = m_sizeY - 1;
    }
    else
    {
        m_cursory = (numLine) ? static_cast<pos_t>(numLine) : 0;
    }

    if (m_xOffset != x || m_firstLine != line)
    {
        m_xOffset = x;
        m_firstLine = line;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveStrBegin([[maybe_unused]]input_t cmd)
{
    size_t curx = m_xOffset + m_cursorx;

    auto str = m_editor->GetStr(m_firstLine + m_cursory);
    size_t x;
    for (x = 0; x < str.size(); ++x)
        if (str[x] > ' ')
            break;

    size_t offset = 0;

    if (x >= curx)
        m_cursorx = 0;
    else if (x < static_cast<size_t>(m_sizeX))
        m_cursorx = static_cast<pos_t>(x);
    else
    {
        m_cursorx = 0;
        offset = x;
    }

    if (m_xOffset != offset)
    {
        m_xOffset = offset;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveStrEnd([[maybe_unused]]input_t cmd)
{
    size_t x;

    auto str = m_editor->GetStr(m_firstLine + m_cursory);
    size_t len = Editor::UStrLen(str);
    if (len >= static_cast<size_t>(m_sizeX))
    {
        m_cursorx = m_sizeX - 1;
        if (len == MAX_STRLEN)
            x = len - m_cursorx - 1;
        else
            x = len - m_cursorx;
    }
    else
    {
        m_cursorx = static_cast<pos_t>(len);
        x = 0;
    }

    if (m_xOffset != x)
    {
        m_xOffset = x;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return true;
}

bool EditorWnd::MoveTabLeft([[maybe_unused]]input_t cmd)
{
    size_t x = m_xOffset + m_cursorx;
    size_t x1 = x - x % 8;
    if (x1 == x)
        x1 -= 8;

    MoveLeft(K_ED(E_MOVE_TAB_LEFT) + static_cast<input_t>(x - x1));

    return true;
}

bool EditorWnd::MoveTabRight([[maybe_unused]]input_t cmd)
{
    size_t x = m_xOffset + m_cursorx;
    size_t x1 = (x + 8) - (x + 8) % 8;

    MoveRight(K_ED(E_MOVE_TAB_RIGHT) + static_cast<input_t>(x1 - x));

    return true;
}

bool EditorWnd::MoveWordLeft([[maybe_unused]]input_t cmd)
{
    size_t x = m_xOffset + m_cursorx;
    size_t line = m_firstLine + m_cursory;
    auto str = m_editor->GetStr(line);

    bool rc{true};

    if (x)
    {
        size_t p = x - 1;//prev char
        auto type = GetSymbolType(str[p]);
        if (type == symbol_t::alnum)
        {
            if (p)
                //goto begin of word
                for (--p; p > 0; --p)
                    if (GetSymbolType(str[p]) != type)
                    {
                        ++p;
                        break;
                    }
        }
        else
        {
            if (p)
                //goto end of word
                for (--p; p > 0; --p)
                {
                    type = GetSymbolType(str[p]);
                    if (type == symbol_t::alnum)
                        break;
                }

            if (p)
            {
                type = GetSymbolType(str[p]);
                //goto begin of word
                for (--p; p > 0; --p)
                {
                    if (GetSymbolType(str[p]) != type)
                    {
                        ++p;
                        break;
                    }
                }
            }
        }

        MoveLeft(K_ED(E_MOVE_LEFT) + static_cast<input_t>(x - p));
    }
    else if (line)
    {
        rc = MoveUp(0);
        rc = MoveStrEnd(0);
    }

    return rc;
}

bool EditorWnd::MoveWordRight([[maybe_unused]]input_t cmd)
{
    size_t x = m_xOffset + m_cursorx;
    size_t line = m_firstLine + m_cursory;
    auto str = m_editor->GetStr(line);

    size_t len = Editor::UStrLen(str);
    bool rc{true};

    if (len > 0 && x < len - 1)
    {
        auto type = GetSymbolType(str[x]);
        size_t p = x;
        if (type == symbol_t::alnum)
        {
            //goto end of word
            for (++p; p < len; ++p)
                if (GetSymbolType(str[p]) != type)
                    break;
        }
        if (p < len)
            //goto begin of word
            for (++p; p < len; ++p)
            {
                type = GetSymbolType(str[p]);
                if (type == symbol_t::alnum)
                    break;
            }

        MoveRight(K_ED(E_MOVE_RIGHT) + static_cast<input_t>(p - x));
    }
    else
    {
        rc = MoveDown(0);
        rc = MoveStrBegin(0);
    }
    return rc;
}

bool EditorWnd::MoveCenter([[maybe_unused]]input_t cmd)
{
    size_t line = m_firstLine + m_cursory;
    if (line > static_cast<size_t>(m_sizeY / 2))
    {
        m_cursory = m_sizeY / 2;
        m_firstLine = line - m_cursory;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }
    return true;
}

bool EditorWnd::SelectBegin(input_t cmd)
{
    //LOG(DEBUG) << "    SelectBegin code " << std::hex << cmd << std::dec << " s=" << m_selectKeyShift;
    if ((m_selectState & select_state::end) != 0)
        SelectUnselect(cmd);

    InputCapture();
    EditorApp::StatusMark((m_selectKeyShift || m_selectMouse) ? EditorApp::mark_status::mark : EditorApp::mark_status::mark_by_key);

    if (!m_selectState)
    {
        //start selection
        m_selectState = select_state::begin;
        m_beginX = m_endX = m_xOffset + m_cursorx;
        m_beginY = m_endY = m_firstLine + m_cursory;

        if (K_GET_CODE(cmd) == 1)
        {
            //select key arrow with key shift
            //LOG(DEBUG) << "    ShiftSelect";
            m_selectKeyShift = true;
        }
        else
            m_selectKeyShift = false;
    }
    else
    {
        //continue selection
        size_t x = m_xOffset + m_cursorx;
        size_t y = m_firstLine + m_cursory;

        if (x == m_endX && y == m_endY)
            return true;

        if (m_selectType == select_t::stream)
        {
            //check for new < prev
            if (IsNormalSelection(m_beginX, m_beginY, m_endX, m_endY))
            {
                if (!IsNormalSelection(m_endX, m_endY, x, y))
                {
                    //TPRINT(("+-\n"));
                    Mark(m_endX, m_endY, x, y, 0, m_selectType);
                }
            }
            else
            {
                if (IsNormalSelection(m_endX, m_endY, x, y))
                {
                    //TPRINT(("-+\n"));
                    Mark(m_endX, m_endY, x, y, 0, m_selectType);
                }
            }
        }
        else if (m_selectType == select_t::line)
        {
            //check for new < prev
            if (IsNormalSelection(0, m_beginY, 0, m_endY))
            {
                if (!IsNormalSelection(0, m_endY, 0, y))
                {
                    //TPRINT(("+-\n"));
                    Mark(0, m_endY, 0, y + 1, 0, m_selectType);
                }
            }
            else
            {
                if (IsNormalSelection(0, m_endY, 0, y))
                {
                    //TPRINT(("-+\n"));
                    Mark(0, m_endY, 0, y - 1, 0, m_selectType);
                }
            }
        }
        else
        {
            //column
            if (m_beginX <= m_endX)
            {
                if (x < m_endX)
                {
                    //TPRINT(("Column clear X1\n"));
                    Mark(x, m_beginY, m_endX, m_endY, 0, m_selectType);
                }
            }
            else
            {
                if (x > m_endX)
                {
                    //TPRINT(("Column clear X2\n"));
                    Mark(x, m_beginY, m_endX, m_endY, 0, m_selectType);
                }
            }

            if (m_beginY <= m_endY)
            {
                if (y < m_endY)
                {
                    //TPRINT(("Column clear Y1\n"));
                    Mark(m_beginX, y, m_endX, m_endY, 0, m_selectType);
                }
            }
            else
            {
                if (y > m_endY)
                {
                    //TPRINT(("Column clear Y2\n"));
                    Mark(m_beginX, y, m_endX, m_endY, 0, m_selectType);
                }
            }
        }

        m_endX = x;
        m_endY = y;

        //show selection
        Mark(m_beginX, m_beginY, m_endX, m_endY, ColorWindowSelect, m_selectType);
        m_selectState = select_state::begin_vis;
    }

    return true;
}

bool EditorWnd::SelectEnd(input_t cmd)
{
    if (m_selectState == select_state::no || IsSelectFinished())
        return 0;

    //LOG(DEBUG) << "    SelectEnd code " << std::hex << cmd << std::dec << " s=" << m_selectKeyShift;
    InputRelease();
    EditorApp::StatusMark();

    if (!m_selectKeyShift && K_GET_CODE(cmd) == 1)
        //not key shift select
        return true;

    m_selectKeyShift = false;

    m_endX = m_xOffset + m_cursorx;
    m_endY = m_firstLine + m_cursory;

    if (m_beginX != m_endX || m_beginY != m_endY)
    {
        //show selection
        Mark(m_beginX, m_beginY, m_endX, m_endY, ColorWindowSelect, m_selectType);
        m_selectState = select_state::complete;
    }
    else if (m_selectType != select_t::line)
    {
        //only 1 symbol selected
        Mark(m_beginX, m_beginY, m_endX, m_endY, 0, m_selectType);

        SelectClear();
    }

    return true;
}

bool EditorWnd::SelectUnselect([[maybe_unused]]input_t cmd)
{
    //LOG(DEBUG) << "    SelectUnselect " << std::hex << cmd << std::dec;
    if (IsSelectVisible() || m_beginX != m_endX || m_beginY != m_endY)
    {
        //hide selection
        Mark(m_beginX, m_beginY, m_endX, m_endY, 0, m_selectType);
    }

    SelectClear();

    return true;
}

bool EditorWnd::SelectMode(input_t cmd)
{
    if (IsSelectStarted())
        SelectEnd(0);
    else
    {
        if (IsSelectFinished())
            SelectUnselect(0);

        input_t type = K_GET_CODE(cmd);
        if (!type)
            m_selectType = select_t::line;
        else
            m_selectType = select_t::column;

        SelectBegin(0);
        //show selection
        Mark(m_beginX, m_beginY, m_endX, m_endY, ColorWindowSelect, m_selectType);
        m_selectState = select_state::begin_vis;
    }
    return true;
}

bool EditorWnd::SelectWord(input_t cmd)
{
    if (IsSelectFinished())
        SelectUnselect(cmd);

    auto str = m_editor->GetStr(m_firstLine + m_cursory);
    size_t begin, end;
    if (!FindWord(str, begin, end))
        return true;

    m_beginY = m_endY = m_firstLine + m_cursory;
    m_beginX = begin;
    m_endX = end;

    //show selection
    m_selectType = select_t::stream;
    m_selectState = select_state::complete;
    Mark(m_beginX, m_beginY, m_endX, m_endY, ColorWindowSelect, m_selectType);

    return true;
}

bool EditorWnd::SelectLine(input_t cmd)
{
    if (IsSelectFinished())
        SelectUnselect(cmd);

    m_beginY = m_endY = m_firstLine + m_cursory;

    m_beginX = 0;
    m_endX = MAX_STRLEN;

    //show selection
    m_selectType = select_t::stream;
    m_selectState = select_state::complete;
    Mark(m_beginX, m_beginY, m_endX, m_endY, ColorWindowSelect, m_selectType);

    return true;
}

bool EditorWnd::SelectAll(input_t cmd)
{
    if (IsSelectFinished())
        SelectUnselect(cmd);

    m_beginY = 0;
    m_endY = m_editor->GetStrCount() - 1;
    m_beginX = 0;
    m_endX = MAX_STRLEN;

    //show selection
    m_selectType = select_t::stream;
    m_selectState = select_state::complete;
    Mark(m_beginX, m_beginY, m_endX, m_endY, ColorWindowSelect, m_selectType);

    return true;
}

bool EditorWnd::MoveLexMatch([[maybe_unused]]input_t cmd)
{
    LOG(DEBUG) << "    MoveLexMatch";

    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    if (m_editor->CheckLexPair(y, x))
        _GotoXY(x, y);
    else
        EditorApp::SetHelpLine("No lexical pair found", stat_color::grayed);

    return true;
}
