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

#define USE_SCROLL

#ifndef WIN32
    #define ONLY_SCREEN_SCROLL
#endif


std::unordered_map<EditorCmd, std::pair<EditorWnd::EditorFunc, EditorWnd::select_state>> EditorWnd::s_funcMap =
{
    {E_MOVE_LEFT,           {&EditorWnd::MoveLeft,               EditorWnd::select_state::begin}},
    {E_MOVE_RIGHT,          {&EditorWnd::MoveRight,              EditorWnd::select_state::begin}},
    {E_MOVE_UP,             {&EditorWnd::MoveUp,                 EditorWnd::select_state::begin}},
    {E_MOVE_DOWN,           {&EditorWnd::MoveDown,               EditorWnd::select_state::begin}},
    {E_MOVE_SCROLL_LEFT,    {&EditorWnd::MoveScrollLeft,         EditorWnd::select_state::begin}},
    {E_MOVE_SCROLL_RIGHT,   {&EditorWnd::MoveScrollRight,        EditorWnd::select_state::begin}},
    {E_MOVE_PAGE_UP,        {&EditorWnd::MovePageUp,             EditorWnd::select_state::begin}},
    {E_MOVE_PAGE_DOWN,      {&EditorWnd::MovePageDown,           EditorWnd::select_state::begin}},
    {E_MOVE_FILE_BEGIN,     {&EditorWnd::MoveFileBegin,          EditorWnd::select_state::begin}},
    {E_MOVE_FILE_END,       {&EditorWnd::MoveFileEnd,            EditorWnd::select_state::begin}},
    {E_MOVE_STR_BEGIN,      {&EditorWnd::MoveStrBegin,           EditorWnd::select_state::begin}},
    {E_MOVE_STR_END,        {&EditorWnd::MoveStrEnd,             EditorWnd::select_state::begin}},
    {E_MOVE_TAB_LEFT,       {&EditorWnd::MoveTabLeft,            EditorWnd::select_state::begin}},
    {E_MOVE_TAB_RIGHT,      {&EditorWnd::MoveTabRight,           EditorWnd::select_state::begin}},
    {E_MOVE_WORD_LEFT,      {&EditorWnd::MoveWordLeft,           EditorWnd::select_state::begin}},
    {E_MOVE_WORD_RIGHT,     {&EditorWnd::MoveWordRight,          EditorWnd::select_state::begin}},
    {E_MOVE_POS,            {&EditorWnd::MovePos,                EditorWnd::select_state::begin}},
    {E_MOVE_CENTER,         {&EditorWnd::MoveCenter,             EditorWnd::select_state::no}},

    {E_SELECT_WORD,         {&EditorWnd::SelectWord,             EditorWnd::select_state::no}},
    {E_SELECT_LINE,         {&EditorWnd::SelectLine,             EditorWnd::select_state::no}},
    {E_SELECT_ALL,          {&EditorWnd::SelectAll,              EditorWnd::select_state::no}},
    {E_SELECT_BEGIN,        {&EditorWnd::SelectBegin,            EditorWnd::select_state::no}},
    {E_SELECT_END,          {&EditorWnd::SelectEnd,              EditorWnd::select_state::no}},
    {E_SELECT_UNSELECT,     {&EditorWnd::SelectUnselect,         EditorWnd::select_state::no}},
    {E_SELECT_MODE,         {&EditorWnd::SelectMode,             EditorWnd::select_state::no}},

    {E_EDIT_C,              {&EditorWnd::EditC,                  EditorWnd::select_state::end}},
    {E_EDIT_DEL_C,          {&EditorWnd::EditDelC,               EditorWnd::select_state::end}},
    {E_EDIT_BS,             {&EditorWnd::EditBS,                 EditorWnd::select_state::end}},
    {E_EDIT_TAB,            {&EditorWnd::EditTab,                EditorWnd::select_state::end}},
    {E_EDIT_ENTER,          {&EditorWnd::EditEnter,              EditorWnd::select_state::end}},
    {E_EDIT_DEL_STR,        {&EditorWnd::EditDelStr,             EditorWnd::select_state::end}},
    {E_EDIT_DEL_BEGIN,      {&EditorWnd::EditDelBegin,           EditorWnd::select_state::end}},
    {E_EDIT_DEL_END,        {&EditorWnd::EditDelEnd,             EditorWnd::select_state::end}},

    {E_EDIT_BLOCK_CLEAR,    {&EditorWnd::EditBlockClear,         EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_COPY,     {&EditorWnd::EditBlockCopy,          EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_MOVE,     {&EditorWnd::EditBlockMove,          EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_DEL,      {&EditorWnd::EditBlockDel,           EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_INDENT,   {&EditorWnd::EditBlockIndent,        EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_UNDENT,   {&EditorWnd::EditBlockUndent,        EditorWnd::select_state::end}},
    {E_EDIT_CB_COPY,        {&EditorWnd::EditCopyToClipboard,    EditorWnd::select_state::end}},
    {E_EDIT_CB_CUT,         {&EditorWnd::EditCutToClipboard,     EditorWnd::select_state::end}},
    {E_EDIT_CB_PASTE,       {&EditorWnd::EditPasteFromClipboard, EditorWnd::select_state::end}},
    {E_EDIT_UNDO,           {&EditorWnd::EditUndo,               EditorWnd::select_state::end}},
    {E_EDIT_REDO,           {&EditorWnd::EditRedo,               EditorWnd::select_state::end}},

    {E_CTRL_FIND,           {&EditorWnd::CtrlFind,               EditorWnd::select_state::begin}},
    {E_CTRL_FINDUP,         {&EditorWnd::CtrlFindUp,             EditorWnd::select_state::begin}},
    {E_CTRL_FINDDN,         {&EditorWnd::CtrlFindDown,           EditorWnd::select_state::begin}},
    {E_CTRL_FINDUPW,        {&EditorWnd::FindUpWord,             EditorWnd::select_state::begin}},
    {E_CTRL_FINDDNW,        {&EditorWnd::FindDownWord,           EditorWnd::select_state::begin}},
    {E_CTRL_REPLACE,        {&EditorWnd::Replace,                EditorWnd::select_state::end}},
    {E_CTRL_REPEAT,         {&EditorWnd::Repeat,                 EditorWnd::select_state::end}},

    {E_DLG_GOTO,            {&EditorWnd::DlgGoto,                EditorWnd::select_state::end}},
    {E_DLG_FIND,            {&EditorWnd::DlgFind,                EditorWnd::select_state::end}},
    {E_DLG_REPLACE,         {&EditorWnd::DlgReplace,             EditorWnd::select_state::end}},

    {E_CTRL_GETSUBSTR,      {&EditorWnd::CtrlGetSubstr,          EditorWnd::select_state::end}},
    {E_CTRL_REFRESH,        {&EditorWnd::CtrlRefresh,            EditorWnd::select_state::end}},
    {E_CTRL_RELOAD,         {&EditorWnd::Reload,                 EditorWnd::select_state::end}},
    {E_CTRL_SAVE,           {&EditorWnd::Save,                   EditorWnd::select_state::end}},
    {E_CTRL_SAVEAS,         {&EditorWnd::SaveAs,                 EditorWnd::select_state::end}},
    {E_CTRL_CLOSE,          {&EditorWnd::Close,                  EditorWnd::select_state::end}},

    {E_MOVE_LEX_MATCH,      {&EditorWnd::MoveLexMatch,           EditorWnd::select_state::begin}},
    {E_CTRL_FLIST,          {&EditorWnd::CtrlFuncList,           EditorWnd::select_state::end}},
    {E_CTRL_PROPERTIES,     {&EditorWnd::CtrlProperties,         EditorWnd::select_state::end}},
    {E_CTRL_CHANGE_CP,      {&EditorWnd::CtrlChangeCP,           EditorWnd::select_state::no}},
    {E_POPUP_MENU,          {&EditorWnd::TrackPopupMenu,         EditorWnd::select_state::end}}
};

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
    size_t step = K_GET_CODE(cmd);
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
            if (offset > step)
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
    size_t step = K_GET_CODE(cmd);
    size_t offset = m_xOffset;

    if (!step)
    {
        if (m_cursorx < m_sizeX - 1)
            ++m_cursorx;
        else if (offset < MAX_STRLEN - m_sizeX)
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

            if (offset < MAX_STRLEN - m_sizeX - step)
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
    size_t step = K_GET_CODE(cmd);
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
        if (line > step)
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
    size_t step = K_GET_CODE(cmd);
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

    size_t offset = m_xOffset - (step ? step : 1);
    if (offset < 0)
        offset = 0;

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

bool EditorWnd::MoveScrollRight(input_t cmd)
{
    size_t step = K_GET_CODE(cmd);

    if (m_xOffset >= MAX_STRLEN - m_sizeX)
        return MoveRight(cmd);

    size_t offset = m_xOffset + (step ? step : 1);
    if (offset > MAX_STRLEN - m_sizeX)
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

    if (line >= m_sizeY - 1)
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
    if (numLine >= m_sizeY)
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
    else if (x < m_sizeX)
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
    if (len >= m_sizeX)
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

bool EditorWnd::MoveWordLeft(input_t cmd)
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

bool EditorWnd::MoveWordRight(input_t cmd)
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

bool EditorWnd::MoveCenter(input_t cmd)
{
    size_t line = m_firstLine + m_cursory;
    if (line > m_sizeY / 2)
    {
        m_cursory = m_sizeY / 2;
        m_firstLine = line - m_cursory;
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }
    return true;
}

bool EditorWnd::SelectBegin(input_t cmd)
{
    LOG(DEBUG) << "    SelectBegin code " << std::hex << cmd << std::dec << " s=" << m_selectKeyShift;
    if ((m_selectState & select_state::end) != 0)
        SelectUnselect(cmd);

    CaptureInput();
    //???StatusMark((m_nSelectKeyShift || m_nSelectMouse) ? 1 : 2);

    if (!m_selectState)
    {
        //start selection
        m_selectState = select_state::begin;
        m_beginX = m_endX = m_xOffset + m_cursorx;
        m_beginY = m_endY = m_firstLine + m_cursory;

        if (K_GET_CODE(cmd) == 1)
        {
            //select key arrow with key shift
            LOG(DEBUG) << "    ShiftSelect";
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

    LOG(DEBUG) << "    SelectEnd code " << std::hex << cmd << std::dec << " s=" << m_selectKeyShift;
    InputRelease();
    //???StatusMark(0);

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

bool EditorWnd::SelectUnselect(input_t cmd)
{
    LOG(DEBUG) << "    SelectUnselect " << std::hex << cmd << std::dec;
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

bool EditorWnd::EditC(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelC(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBS(input_t cmd)
{
    return true;
}

bool EditorWnd::EditTab(input_t cmd)
{
    return true;
}

bool EditorWnd::EditEnter(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelStr(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelBegin(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelEnd(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockClear(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockCopy(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockMove(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockDel(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockIndent(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockUndent(input_t cmd)
{
    return true;
}

bool EditorWnd::EditCopyToClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditCutToClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditPasteFromClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditUndo(input_t cmd)
{
    return true;
}

bool EditorWnd::EditRedo(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFind(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFindUp(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFindDown(input_t cmd)
{
    return true;
}

bool EditorWnd::FindUpWord(input_t cmd)
{
    return true;
}

bool EditorWnd::FindDownWord(input_t cmd)
{
    return true;
}

bool EditorWnd::Replace(input_t cmd)
{
    return true;
}

bool EditorWnd::Repeat(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgGoto(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgFind(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgReplace(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlGetSubstr(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlRefresh(input_t cmd)
{
    return true;
}

bool EditorWnd::Reload(input_t cmd)
{
    return true;
}

bool EditorWnd::Save(input_t cmd)
{
    return true;
}

bool EditorWnd::SaveAs(input_t cmd)
{
    return true;
}

bool EditorWnd::Close(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveLexMatch(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFuncList(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlProperties(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlChangeCP(input_t cmd)
{
    return true;
}

bool EditorWnd::TrackPopupMenu(input_t cmd)
{
    return true;
}

