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
#include "utils/logger.h"
#include "utfcpp/utf8.h"
#include "utils/SymbolType.h"
#include "utils/CpConverter.h"
#include "EditorWnd.h"
#include "WndManager/WndManager.h"
#include "EditorApp.h"
#include "WndManager/Dialog.h"
#include "Dialogs/EditorDialogs.h"

#include <algorithm>
#include <iterator>
#include <cwctype>

namespace _Editor
{

bool EditorWnd::SetFileName(const std::filesystem::path& file, bool untitled, const std::string& parseMode, const std::string& cp)
{
    //LOG(DEBUG) << "    SetFileName '" << file.u8string() << "'";

    m_untitled = untitled;
    auto editor = std::make_shared<Editor>(file, parseMode, cp);
    if (!untitled)
    {
        bool rc = editor->Load();
        if (!rc)
        {
            MsgBox(MBoxKey::OK, "Load",
                { "File load error",
                "Check file and try again" }
            );
            return rc;
        }
    }

    return SetEditor(editor);
}

bool EditorWnd::SetEditor(EditorPtr editor) 
{ 
    //LOG(DEBUG) << "    SetTextBuff";
    if (m_editor)
    {
        Save(K_ED(E_CTRL_SAVE));
        m_editor->UnlinkWnd(this);
        m_editor = nullptr;
    }

    m_editor = editor;
    m_editor->LinkWnd(this);

    m_saved = false;

    m_cursorx = 0;
    m_cursory = 0;

    m_xOffset = 0;
    m_firstLine = 0;
    m_clientSizeX = 0;
    m_clientSizeY = 0;

    m_selectState = select_state::no;
    m_beginX = 0;
    m_beginY = 0;
    m_endX = 0;
    m_endY = 0;

    m_foundX = 0;
    m_foundY = 0;
    m_foundSize = 0;

    Refresh();

    return true; 
}

bool EditorWnd::Refresh()
{
    if (!WndManager::getInstance().IsVisible(this))
        return true;

    m_clientSizeX = GetCSizeX();
    m_clientSizeY = GetCSizeY();
    //LOG(DEBUG) << "    EditorWnd::Refresh " << this;
    //LOG(DEBUG) << "sx=" << m_clientSizeX << " sy=" << m_clientSizeY << " cx=" << m_cursorx << " cy=" << m_cursory;

    if (m_clientSizeX <= m_cursorx || m_clientSizeY <= m_cursory)
        _GotoXY(m_xOffset + m_cursorx, m_firstLine + m_cursory);

    bool rc = DrawBorder();

    if (!m_editor)
    {
        rc = Clr();
        return rc;
    }

    UpdateNameInfo();

    InvalidateRect(0, 0, m_clientSizeX, m_clientSizeY);
    Repaint();

    UpdateLexPair();

    return rc;
}

bool EditorWnd::UpdateAccessInfo()
{
    pos_t x = 0;
    if (m_border & BORDER_TOP)
        x += 2;

    std::string info{ "[  ]" };
    info[2] = m_readOnly ? 'R' : GetAccessInfo();

    WriteWnd(x, 0, info, ColorWindowInfo);
    return true;
}

bool EditorWnd::UpdateNameInfo()
{
    if (m_clientSizeX / 2 - 2 > 5)
    {
        pos_t x = 5;
        if (m_border & BORDER_TOP)
            x += 2;

        auto path = m_editor->GetFilePath().filename().u8string();
        path.resize(m_clientSizeX / 2 - 2, ' ');
        WriteWnd(x, 0, path, *m_pColorWindowTitle);
    }
    return true;
}

bool EditorWnd::UpdatePosInfo()
{
    size_t x = m_xOffset + m_cursorx + 1;
    size_t y = m_firstLine + m_cursory + 1;

    std::stringstream stream;

    //15 spaces
    if (m_clientSizeX > 40)
        stream << "               [" << m_editor->GetStrCount() << "] L:" << y << " C:" << x;
    else
        stream << "               L:" << y << " C:" << x;

    auto str = stream.str();

    size_t len = std::max(str.size(), m_infoStrSize);
    len = std::min({len, static_cast<size_t>(32), static_cast<size_t>(m_clientSizeX / 2 + 3)});

    m_infoStrSize = str.size() - 14;

    WriteWnd(m_clientSizeX - static_cast<pos_t>(len), 0, str.substr(str.size() - len), *m_pColorWindowTitle);
    return true;
}

bool EditorWnd::_GotoXY(size_t x, size_t y, bool top)
{
    if (m_clientSizeX == 0 || m_clientSizeY == 0)
    {
        m_clientSizeX = GetCSizeX();
        m_clientSizeY = GetCSizeY();
    }

    size_t line = 0;
    size_t pos = 0;

    if (y > m_editor->GetStrCount())
        y = m_editor->GetStrCount();

    if (x == 0)
        m_cursorx = 0;
    else if (x < static_cast<size_t>(m_clientSizeX - 1))
        m_cursorx = static_cast<pos_t>(x);
    else
    {
        m_cursorx = m_clientSizeX - 5;//we will see 4 symbols
        pos = x - m_cursorx;
        if (pos >= static_cast<size_t>(m_editor->GetMaxStrLen() - m_clientSizeX))
        {
            pos = m_editor->GetMaxStrLen() - m_clientSizeX;
            m_cursorx = static_cast<pos_t>(x - pos);
        }
    }

    if (!top)
    {
        if (y <= 0)
            m_cursory = 0;
        else if (y >= m_firstLine && y < m_firstLine + m_clientSizeY)
        {
            line = m_firstLine;
            m_cursory = static_cast<pos_t>(y - line);
        }
        else if (y < static_cast<size_t>(m_clientSizeY / 2))
            m_cursory = static_cast<pos_t>(y);
        else
        {
            m_cursory = m_clientSizeY / 2;
            line = y - m_cursory;
        }
    }
    else
    {
        m_cursory = 0;
        line = y;
    }

    bool rc = true;
    if (m_xOffset != pos || m_firstLine != line)
    {
        m_xOffset = pos;
        m_firstLine = line;
        rc = InvalidateRect(0, 0, m_clientSizeX, m_clientSizeY);
    }

    return rc;
}

bool EditorWnd::InvalidateRect(pos_t x, pos_t y, pos_t sizex, pos_t sizey)
{
    if (0 == m_clientSizeX || 0 == m_clientSizeY)
    {
        m_clientSizeX = GetCSizeX();
        m_clientSizeY = GetCSizeY();
    }

    if (sizex == 0)
        sizex = m_clientSizeX;
    if (sizey == 0)
        sizey = m_clientSizeY;

    m_invalidate = true;

    if (m_invBeginX == m_invEndX)
    {
        m_invBeginX = x;
        m_invBeginY = y;
        m_invEndX = x + sizex;
        m_invEndY = y + sizey;
    }
    else
    {
        if (x < m_invBeginX)
            m_invBeginX = x;
        if (y < m_invBeginY)
            m_invBeginY = y;

        pos_t endx = x + sizex;
        pos_t endy = y + sizey;

        if (endx > m_invEndX)
            m_invEndX = endx;
        if (endy > m_invEndY)
            m_invEndY = endy;
    }
    return true;
}

bool EditorWnd::Invalidate(size_t line, invalidate_t type, size_t pos, size_t size)
{
    if (size == 0)
        size = m_editor->GetMaxStrLen();

    int y = static_cast<int>(line) - static_cast<int>(m_firstLine);
    int x = static_cast<int>(pos) - static_cast<int>(m_xOffset);

    int endx = x + static_cast<int>(size);
    if (endx >= m_clientSizeX)
        endx = m_clientSizeX;

    if (x < 0 && endx > 0)
        x = 0;

    //LOG(DEBUG) << "    Invalidate line=" << line << " type=" << static_cast<int>(type) << " pos=" << pos << " size=" << size << "; y=" << y << " x=" << x << " endx=" << endx;

    if (y < 0 && type != invalidate_t::find && type != invalidate_t::change)
    {
        y = 0;
        type = invalidate_t::full;
    }

    if (y < 0 || y >= m_clientSizeY || x < 0 || x >= m_clientSizeX || endx < 0)
        return true;

    switch (type)
    {
    case invalidate_t::find:
    case invalidate_t::change:
        InvalidateRect(static_cast<pos_t>(x), static_cast<pos_t>(y), static_cast<pos_t>(endx - x), 1);
        break;
    case invalidate_t::del:
        InvalidateRect(0, static_cast<pos_t>(y), m_clientSizeX, static_cast<pos_t>(m_clientSizeY - y));
        break;
    case invalidate_t::insert:
        InvalidateRect(0, static_cast<pos_t>(y), m_clientSizeX, static_cast<pos_t>(m_clientSizeY - y));
        break;
    case invalidate_t::full:
        InvalidateRect(0, 0, m_clientSizeX, m_clientSizeY);
        break;
    }

    return true;
}

bool EditorWnd::Repaint()
{
    if (!WndManager::getInstance().IsVisible(this))
        return true;

    bool rc;
    if (m_invalidate)
    {
        if (m_invBeginX > m_clientSizeX)
            m_invBeginX = 0;
        if (m_invEndX > m_clientSizeX)
            m_invEndX = m_clientSizeX;

        if (m_invBeginY > m_clientSizeY)
            m_invBeginY = 0;
        if (m_invEndY > m_clientSizeY)
            m_invEndY = m_clientSizeY;

        StopPaint();

        for (pos_t i = m_invBeginY; i < m_invEndY; ++i)
        {
            auto str = m_editor->GetStr(m_firstLine + i);
            if (str.size() < m_xOffset + m_invEndX)
                str.resize(m_xOffset + m_invEndX, ' ');
            rc = PrintStr(m_invBeginX, i, str, m_xOffset + m_invBeginX, m_invEndX - m_invBeginX);
        }

        BeginPaint();
        rc = ShowBuff(m_invBeginX, m_invBeginY, m_invEndX - m_invBeginX, m_invEndY - m_invBeginY);

        m_invalidate = false;
        m_invBeginX = 0;
        m_invBeginY = 0;
        m_invEndX = 0;
        m_invEndY = 0;
    }

    StopPaint();
    UpdateAccessInfo();
    UpdatePosInfo();
    BeginPaint();
    //-1 for updating window caption
    rc = ShowBuff(0, static_cast<pos_t>(-1), m_clientSizeX, 1);

    return rc;
}

bool EditorWnd::IsNormalSelection(size_t bx, size_t by, size_t ex, size_t ey) const
{
    //true - normal selection
    //false - inverse selection
    if (by == ey)
    {
        if (bx <= ex)
            return true;
        else
            return false;
    }
    else if (by < ey)
        return true;
    else
        return false;
}

bool EditorWnd::PrintStr(pos_t x, pos_t y, const std::u16string& wstr, size_t offset, size_t len)
{
    size_t line = y + m_firstLine;
    auto MarkFound = [this, line]() {
        if (m_foundSize && line == m_foundY)
            //mark found
            Mark(m_foundX, m_foundY, m_foundX + m_foundSize - 1, m_foundY, ColorWindowFound);
    };

    auto str = iconvpp::CpConverter::FixPrintWidth(wstr, len);
    bool rc{};
    if (!m_diff)
    {
        std::vector<color_t> colorBuff;
        rc = m_editor->GetColor(m_firstLine + y, str, colorBuff, offset + len);

        if (rc)
            rc = WriteColorStr(x, y, str.substr(offset, len), std::vector<color_t>(colorBuff.cbegin() + offset, colorBuff.cend()));
        else
            rc = WriteWStr(x, y, str.substr(offset, len));
    }
    else
    {
/* //???
        if (m_diff->IsDiff(m_nDiffBuff, m_nFirstLine + y))//???
            SetTextAttr(ColorWindowDiff);
        else
            SetTextAttr(ColorWindowNotDiff);
        rc = WriteWStr(x, y, pStr + offset);
*/
    }

    if (!IsSelectVisible())
    {
        //not selected
        MarkFound();
        return true;
    }

    size_t bx, ex;
    [[maybe_unused]]select_line type;
    if (!GetSelectedPos(line, bx, ex, type))
    {
        //line not selected
        MarkFound();
        return true;
    }

    if (!m_diff)
        rc = Mark(bx, line, ex, line, ColorWindowSelect, m_selectType);
    else
        rc = Mark(bx, line, ex, line, ColorWindowCurDiff, m_selectType);

    MarkFound();

    return rc;
}

bool EditorWnd::Mark(size_t bx, size_t by, size_t ex, size_t ey, color_t color, select_t selectType)
{
    //LOG(DEBUG) << "    Mark bx=" << bx << " by=" << by << " ex=" << ex << " ey=" << ey 
    //    << " color=" << std::hex << color << std::dec << " select=" << static_cast<int>(selectType);

    //check begin and end of selection
    size_t x1, x2;
    size_t y1, y2;

    if (selectType != select_t::column)
    {
        if (IsNormalSelection(bx, by, ex, ey))
        {
            x1 = bx;
            x2 = ex;
            y1 = by;
            y2 = ey;
        }
        else
        {
            x1 = ex;
            x2 = bx;
            y1 = ey;
            y2 = by;
        }

        if (y2 < m_firstLine || y1 >= m_firstLine + m_clientSizeY)
            //not visible
            return true;

        if (y1 < m_firstLine)
        {
            y1 = m_firstLine;
            x1 = 0;
        }
        if (y2 >= m_firstLine + m_clientSizeY)
        {
            y2 = m_firstLine + m_clientSizeY - 1;
            x2 = m_editor->GetMaxStrLen();
        }
    }
    else
    {
        //column
        if (bx <= ex)
        {
            x1 = bx;
            x2 = ex;
        }
        else
        {
            x1 = ex;
            x2 = bx;
        }

        if (by <= ey)
        {
            y1 = by;
            y2 = ey;
        }
        else
        {
            y1 = ey;
            y2 = by;
        }

        if (y2 < m_firstLine || y1 >= m_firstLine + m_clientSizeY)
            //not visible
            return true;

        if (y1 < m_firstLine)
            y1 = m_firstLine;

        if (y2 >= m_firstLine + m_clientSizeY)
            y2 = m_firstLine + m_clientSizeY - 1;
    }

    //LOG(DEBUG) << "    Mark1 bx=" << x1 << " by=" << y1 << " ex=" << x2 << " ey=" << y2;

    for (size_t y = y1; y <= y2; ++y)
    {
        if (selectType == select_t::stream)
        {
            //stream
            if (y == y1)
            {
                //first line
                if (y1 == y2)
                {
                    //fill x1-x2
                    bx = x1;
                    ex = x2;
                }
                else
                {
                    //fill x1-.
                    bx = x1;
                    ex = m_editor->GetMaxStrLen();
                }
            }
            else if (y == y2)
            {
                //last line
                //fill 0-x2
                bx = 0;
                ex = x2;
            }
            else
            {
                //full line
                //fill 0-.
                bx = 0;
                ex = m_editor->GetMaxStrLen();
            }
        }
        else if (selectType == select_t::line)
        {
            //line
            //fill 0-.
            bx = 0;
            ex = m_editor->GetMaxStrLen();
        }
        else
        {
            //column
            //fill x1-x2
            bx = x1;
            ex = x2;
        }

        if (ex < m_xOffset || bx >= m_xOffset + m_clientSizeX)
            //not visible
            continue;
        if (bx < m_xOffset)
            bx = m_xOffset;
        if (ex >= m_xOffset + m_clientSizeX)
            ex = m_xOffset + m_clientSizeX - 1;

        if (color)
            ColorRect(static_cast<pos_t>(bx - m_xOffset), static_cast<pos_t>(y - m_firstLine), static_cast<pos_t>(ex - bx + 1), 1, color);
        else
        {
            auto str = m_editor->GetStr(y);
            std::vector<color_t> colorBuff;
            bool rc = m_editor->GetColor(y, str, colorBuff, ex + 1);
            if (rc)
            {
                //LOG(DEBUG) << "WriteColor bx=" << bx << " ex=" << ex << " y=" << y << " str=" << utf8::utf16to8(str);
                WriteColor(static_cast<pos_t>(bx - m_xOffset), static_cast<pos_t>(y - m_firstLine), std::vector<color_t>(colorBuff.cbegin() + bx, colorBuff.cend()));
            }
            else
                ColorRect(static_cast<pos_t>(bx - m_xOffset), static_cast<pos_t>(y - m_firstLine), static_cast<pos_t>(ex - bx + 1), 1, *m_pColorWindow);
        }
    }

    return true;
}

input_t EditorWnd::EventProc(input_t code)
{
    //LOG(DEBUG) << "    EditorWnd::WndProc " << std::hex << code << std::dec;
    if (code == K_TIME)
    {
        //check for file changing by external program
        if (WndManager::getInstance().IsVisible(this))
            CheckFileChanging();
    }

    if ( code != K_TIME
      && code != K_EXIT
      && code != K_RELEASE + K_SHIFT
      && code != (K_ED(E_CTRL_FIND) | 1) 
      && !m_close)
        //hide search if Dialog window is visible
        HideFound();

    auto scan = m_cmdParser.ScanKey(code);
    if (scan == scancmd_t::wait_next)
        InputCapture();
    else if (scan == scancmd_t::not_found)
        ParseCommand(code);
    else
    {
        InputRelease();
        auto cmdlist = m_cmdParser.GetCommand();
        for(auto cmd : cmdlist)
            ParseCommand(cmd);
    }

    if (code != K_TIME
     && code != K_FOCUSLOST
     && code != K_FOCUSSET)
    {
        Repaint();

        UpdateLexPair();
        m_editor->SetUndoRemark("");

        //refresh all view
        m_editor->RefreshAllWnd(this);
    }

    if (m_close)
    {
        m_close = false;
        Application::getInstance().CloseWindow(this);
        return K_CLOSE;
    }

    //WndManager::getInstance().ShowBuff();//!!! for debug only
    return 0;
}

input_t EditorWnd::ParseCommand(input_t cmd)
{
    if (cmd == K_TIME)
        return 0;
    else if (0 != (cmd & K_MOUSE))
    {
        pos_t x = K_GET_X(cmd);
        pos_t y = K_GET_Y(cmd);
        ScreenToClient(x, y);
        cmd = K_MAKE_COORD_CODE((cmd & ~K_CODEMASK), x, y);

        //mouse event
        if ((cmd & K_TYPEMASK) == K_MOUSE)
            return 0;
        else if ((cmd & K_TYPEMASK) == K_MOUSEKUP)
        {
            m_selectMouse = false;
            InputRelease();
            MovePos(cmd);
            SelectEnd(0);

            if (m_popupMenu)
            {
                m_popupMenu = false;
                TrackPopupMenu(0);
            }
        }
        else
        {
            InputCapture();
            m_selectMouse = true;
            if ((cmd & K_TYPEMASK) == K_MOUSEKL)
            {
                //mouse left key
                if ((cmd & K_MODMASK) == K_MOUSE2)
                {
                    SelectWord(cmd);
                }
                else if ((cmd & K_MODMASK) == K_MOUSE3)
                {
                    SelectLine(cmd);
                }
                else
                {
                    MovePos(cmd);
                    SelectBegin(0);
                    if (m_selectState == select_state::begin)
                    {
                        if (0 != (cmd & K_CTRL))
                            m_selectType = select_t::column;
                        else if (0 != (cmd & K_SHIFT))
                            m_selectType = select_t::line;
                    }
                }
            }
            else if ((cmd & K_TYPEMASK) == K_MOUSEKR && (cmd & (K_SHIFT | K_CTRL | K_ALT)) == 0)
            {
                m_popupMenu = true;
            }
        }
    }
    else if ((cmd & K_TYPEMASK) == 0 && ((cmd & K_MODMASK) & ~K_SHIFT) == 0)
    {
        if (m_selectMouse)
            return 0;

        PutMacro(cmd);

        SelectEnd(cmd);
        switch (cmd & K_CODEMASK)
        {
        case K_ESC:
            break;
        default:
            EditC(cmd);
            break;
        }
    }
    else if(0 != (cmd &  EDITOR_CMD))
    {
        EditorCmd ecmd = static_cast<EditorCmd>((cmd - EDITOR_CMD) >> 16);

        if (m_selectMouse)
        {
            //LOG(DEBUG) << "Mouse select for " << ecmd;
            if (ecmd == E_SELECT_MODE)
            {
                m_selectMouse = false;
                SelectEnd(cmd);
            }
            return 0;
        }

        auto it = s_funcMap.find(ecmd);
        if(it == s_funcMap.end())
        {
            LOG(ERROR) << __FUNC__ << "    ??? editor cmd=" << std::hex << cmd << std::dec;
        }
        else
        {
            PutMacro(cmd);

            auto& [func, select] = it->second;

            if (select == select_state::end)
            {
                SelectEnd(cmd);
            }

            [[maybe_unused]]bool rc = func(this, cmd);

            if (select == select_state::begin && IsSelectStarted())
            {
                SelectBegin(cmd);
            }
        }
    }
    else
    {
        if(cmd != K_FOCUSLOST && cmd != K_FOCUSSET)
            LOG(ERROR) << __FUNC__ << "    ??? editor code=" << std::hex << cmd << std::dec;
    }

    return 0;
}

bool EditorWnd::HideFound()
{
    bool rc = true;
    if (m_lexX >= 0 && m_lexY >= 0)
    {
        //LOG(DEBUG) << "    HideLex x=" << m_lexX << " y=" << m_lexY;
        if ( static_cast<size_t>(m_lexY) >= m_firstLine && static_cast<size_t>(m_lexY) < m_firstLine + m_clientSizeY
          && static_cast<size_t>(m_lexX) >= m_xOffset   && static_cast<size_t>(m_lexX) < m_xOffset + m_clientSizeX)
        {
            //if visible
            auto str = m_editor->GetStr(m_lexY);
            rc = PrintStr(static_cast<pos_t>(m_lexX - m_xOffset), static_cast<pos_t>(m_lexY - m_firstLine), str, m_lexX, 1);
        }

        m_lexX = -1;
        m_lexY = -1;
    }

    if (m_foundSize)
    {
        //LOG(DEBUG) << "    HideFound x=" << m_foundX << " y=" << m_foundY << " size=" << m_foundSize;
        size_t size = m_foundSize;
        m_foundSize = 0;

        if (m_foundY >= m_firstLine && m_foundY < m_firstLine + m_clientSizeY)
        {
            auto str = m_editor->GetStr(m_foundY);

            int x = static_cast<int>(m_foundX) - static_cast<int>(m_xOffset);
            if (x < 0)
            {
                size += x;
                x = 0;
            }

            if (static_cast<size_t>(x + size) > static_cast<size_t>(m_clientSizeX))
            {
                size -= x + size - m_clientSizeX;
            }

            if (size > 0)
                rc = PrintStr(static_cast<pos_t>(x), static_cast<pos_t>(m_foundY - m_firstLine), str, x + m_xOffset, size);
        }
    }
    return rc;
}

bool EditorWnd::SelectClear()
{
    if (m_selectState != select_state::no)
    {
        EditorApp::StatusMark();

        m_selectState = select_state::no;
        m_beginX = m_endX = 0;
        m_beginY = m_endY = 0;
        m_selectType = select_t::stream;
    }

    return true;
}

bool EditorWnd::FindWord(const std::u16string& str, size_t& begin, size_t& end)
{
    size_t x = m_xOffset + m_cursorx;
    if (x > str.size() || str[x] == ' ')
        return false;

    auto type = GetSymbolType(str[x]);

    int b = static_cast<int>(x);
    for (; b >= 0; --b)
        if (GetSymbolType(str[b]) != type)
            break;
    ++b;

    size_t e;
    for (e = x; e < m_editor->GetMaxStrLen(); ++e)
        if (GetSymbolType(str[e]) != type)
            break;
    --e;

    begin = static_cast<size_t>(b);
    end = e;
    
    return true;
}

bool EditorWnd::ChangeSelected(select_change type, size_t line, size_t pos, size_t size)
{
    if (m_selectState != select_state::complete)
        return true;

    //LOG(DEBUG) << "    ChangeSelected type=" << static_cast<int>(type);
    CorrectSelection();

    switch (type)
    {
    case select_change::clear: //clear
        m_beginX = m_endX = 0;
        m_beginY = m_endY = 0;
        m_selectType = select_t::stream;
        m_selectState = select_state::no;
        break;

    case select_change::split_str:
        if (line == m_beginY)
        {
            if (m_selectType != select_t::line)//stream column
            {
                if (pos <= m_beginX)
                {
                    ++m_beginY;
                    if (m_selectType == select_t::stream)
                        m_beginX += size - pos;
                }
            }
        }
        else if (line < m_beginY)
            ++m_beginY;

        if (line == m_endY)
        {
            if (m_selectType != select_t::line)//stream column
            {
                if (pos <= m_endX)
                {
                    ++m_endY;
                    if (m_selectType == select_t::stream)
                        m_endX += size - pos;
                }
            }
            else
                ++m_endY;
        }
        else if (line < m_endY)
            ++m_endY;
        break;

    case select_change::merge_str:
        if (line < m_beginY)
            --m_beginY;

        if (line < m_endY)
            --m_endY;

        if (m_selectType == select_t::stream)
        {
            size = m_endX - m_beginX;
            m_beginX = pos;
            m_endX = pos + size;
        }
        break;

    case select_change::insert_str:
        if (line <= m_beginY)
            ++m_beginY;

        if (line <= m_endY)
            ++m_endY;
        break;

    case select_change::delete_str:
        if (line == m_beginY && line == m_endY)
        {
            //delete marked line
            m_beginX = m_endX = 0;
            m_beginY = m_endY = 0;
            m_selectType = select_t::stream;
            m_selectState = select_state::no;
            //LOG(DEBUG) << "End mark";
        }
        else
        {
            if (line < m_beginY)
                --m_beginY;

            if (line <= m_endY)
                --m_endY;
        }
        break;

    case select_change::insert_ch:
        if (m_selectType == select_t::stream)
        {
            if (line == m_beginY)
                if (pos <= m_beginX)
                    m_beginX += size;

            if (line == m_endY)
                if (pos <= m_endX)
                    m_endX += size;
        }
        break;

    case select_change::delete_ch:
        if (m_selectType == select_t::stream)
        {
            if (line == m_beginY && line == m_endY
                && pos <= m_beginX && pos + size >= m_endX)
            {
                //delete marked ch
                m_beginX = m_endX = 0;
                m_beginY = m_endY = 0;
                m_selectType = select_t::stream;
                m_selectState = select_state::no;
                InvalidateRect();
                //LOG(DEBUG) << "End mark";
            }
            else
            {
                if (line == m_beginY && pos <= m_beginX)
                {
                    if (pos + size < m_beginX)
                        m_beginX -= size;
                    else
                        m_beginX = pos;
                }

                if (line == m_endY && pos <= m_endX)
                {
                    if (pos + size <= m_endX)
                        m_endX -= size;
                    else
                        m_endX = pos - 1;
                }
            }
        }
        break;
    }

    return true;
}

bool EditorWnd::CorrectSelection()
{
    bool rev{};
    if (m_beginY > m_endY)
    {
        rev = true;
        std::swap(m_beginY, m_endY);
    }

    if (m_selectType == select_t::stream)
    {
        if (rev || (m_beginY == m_endY && m_beginX > m_endX))
        {
            std::swap(m_beginX, m_endX);
        }
    }
    else if (m_selectType == select_t::line)
    {
        m_beginX = 0;
        m_endX = m_editor->GetMaxStrLen();
    }
    else
    {
        //column
        if (m_beginX > m_endX)
        {
            std::swap(m_beginX, m_endX);
        }
    }

    return true;
}

bool EditorWnd::GetSelectedPos(size_t line, size_t& begin, size_t& end, select_line& type) const
{
    //check begin and end of selection
    size_t x1, x2;
    size_t y1, y2;

    if (IsNormalSelection(m_beginX, m_beginY, m_endX, m_endY))
    {
        x1 = m_beginX;
        x2 = m_endX;
        y1 = m_beginY;
        y2 = m_endY;
    }
    else
    {
        x1 = m_endX;
        x2 = m_beginX;
        y1 = m_endY;
        y2 = m_beginY;
    }

    if (line < y1 || line > y2)
        return false;

    if (m_selectType == select_t::stream)
    {
        //stream
        if (line == y1)
        {
            //first line
            if (y1 == y2)
            {
                //[x1,x2]
                begin = x1;
                end = x2;
                type = select_line::substr;
            }
            else
            {
                //[x1,).
                begin = x1;
                end = m_editor->GetMaxStrLen();
                type = select_line::end;
            }
        }
        else if (line == y2)
        {
            //last line (,x2]
            begin = 0;
            end = x2;
            type = select_line::begin;
        }
        else
        {
            //[0,)
            begin = 0;
            end = m_editor->GetMaxStrLen();
            type = select_line::full;
        }
    }
    else if (m_selectType == select_t::line)
    {
        //line [0,)
        begin = 0;
        end = m_editor->GetMaxStrLen();
        type = select_line::full;
    }
    else
    {
        //columns [x1,x2]
        if (x1 > x2)
            std::swap(x1, x2);

        begin = x1;
        end = x2;
        type = select_line::substr;
    }

    return true;
}

bool EditorWnd::InsertStr(const std::u16string& str, size_t y, bool save)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    InsertStr";
    if (y == STR_NOTDEFINED)
        y = m_firstLine + m_cursory;

    bool rc = m_editor->AddLine(save, y, str);
    if (!rc)
    {
        //something wrong
        Beep();
    }

    return rc;
}

bool EditorWnd::CopySelected(std::vector<std::u16string>& strArray, select_t& selType)
{
    LOG(DEBUG) << "    CopySelected";

    CorrectSelection();
    strArray.clear();

    size_t n = m_endY - m_beginY;
    for (size_t i = 0; i <= n; ++i)
    {
        size_t bx, ex;
        [[maybe_unused]] select_line type;
        GetSelectedPos(m_beginY + i, bx, ex, type);

        size_t srcY = m_beginY + i;
        size_t size = ex - bx + 1;
        
        auto str = m_editor->GetStr(srcY, bx, size);
        strArray.push_back(str.substr(0, Editor::UStrLen(str)));
    }

    selType = m_selectType;

    return true;
}

bool EditorWnd::PasteSelected(const std::vector<std::u16string>& strArray, select_t selType)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    PasteSelected";

    size_t posX = m_xOffset + m_cursorx;
    size_t posY = m_firstLine + m_cursory;

    EditCmd edit { cmd_t::CMD_BEGIN, posY, posX };
    EditCmd undo { cmd_t::CMD_BEGIN, posY, posX };
    m_editor->SetUndoRemark("Paste block");
    m_editor->AddUndoCommand(edit, undo);

    size_t n = strArray.size();
    bool save{true};

    bool rc{true};
    size_t bx1, ex1{};
    for(size_t i = 0; i < n; ++i)
    {
        const auto& str = strArray[i];

        int copyLine{};

        if (selType == select_t::stream)
        {
            if (n == 1)
            {
                //only 1 string
                bx1 = posX;
                ex1 = posX + str.size();
            }
            else if (i == 0)
            {
                //first line
                bx1 = posX;
                ex1 = m_editor->GetMaxStrLen();
                copyLine = 1;
            }
            else if (i == n - 1)
            {
                //last line
                bx1 = 0;
                ex1 = str.size();
            }
            else
            {
                bx1 = 0;
                ex1 = m_editor->GetMaxStrLen();
                copyLine = 2;
            }
        }
        else if (selType == select_t::line)
        {
            bx1 = 0;
            ex1 = m_editor->GetMaxStrLen();
            copyLine = 2;
        }
        else
        {
            bx1 = posX;
            ex1 = posX + str.size();
        }

        if (ex1 > 0)
            --ex1;

        size_t dstY = posY + i;

        if (copyLine == 1)
        {
            //LOG(DEBUG) << "     Copy first line dy=" << dstY;
            //split and change first line
            if (!bx1)
            {
                rc = InsertStr(str, dstY);
            }
            else
            {
                //split line
                rc = m_editor->SplitLine(save, dstY, bx1);
                rc = m_editor->AddSubstr(save, dstY, bx1, str);
            }
        }
        else if (copyLine == 2)
        {
            //LOG(DEBUG) << "     Copy line dy=" << dstY;
            //insert full line
            rc = InsertStr(str, dstY, save);
        }
        else
        {
            //LOG(DEBUG) << "     Copy substr dx=" << bx1 << " dy=" << dstY;
            //change line
            m_editor->AddSubstr(save, dstY, bx1, str);
        }
    }

    edit.command = cmd_t::CMD_END;
    undo.command = cmd_t::CMD_END;
    m_editor->AddUndoCommand(edit, undo);

#if 0
    //new selection
    m_beginX = posX;
    m_beginY = posY;
    m_endX = ex1;
    m_endY = posY + n - 1;
    m_selectType = selType;
    m_selectState = select_state::complete;
#else
    m_selectState = select_state::no;
#endif

    InvalidateRect();
    return rc;
}

bool EditorWnd::DelSelected()
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    DelSelected";

    CorrectSelection();

    EditCmd edit { cmd_t::CMD_BEGIN, m_beginY, m_beginX };
    EditCmd undo { cmd_t::CMD_BEGIN, m_beginY, m_beginX };
    m_editor->SetUndoRemark("Delete block");
    m_editor->AddUndoCommand(edit, undo);

    size_t n = m_endY - m_beginY;
    bool save{true};
    bool rc{true};

    size_t bx, ex;
    size_t dy = 0;

    for (size_t i = 0; i <= n; ++i)
    {
        select_line type;
        GetSelectedPos(m_beginY + i, bx, ex, type);

        size_t srcY = m_beginY + i;
        size_t size = ex - bx + 1;

        if (type == select_line::end)
        {
            //LOG(DEBUG) << "     Del first line dy=" << m_beginY;
            dy = 1;
            rc = m_editor->DelEnd(save, m_beginY, bx);
            auto str = m_editor->GetStr(m_beginY + n, m_endX + 1);
            rc = m_editor->ChangeSubstr(save, m_beginY, bx, str);
        }
        else if (type != select_line::substr)
        {
            //LOG(DEBUG) << "     Del line dy=" << m_beginY + dy;
            //del full line
            rc = m_editor->DelLine(save, m_beginY + dy);
        }
        else
        {
            //LOG(DEBUG) << "     Del substr dx=" << bx << " dy=" << srcY;
            //change line
            rc = m_editor->DelSubstr(save, srcY, bx, size);
        }
    }

    edit.command = cmd_t::CMD_END;
    undo.command = cmd_t::CMD_END;
    m_editor->AddUndoCommand(edit, undo);

    //correct cursor position
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    if (y >= m_beginY)
    {
        if (m_selectType == select_t::stream)
        {
            //stream
            if (y == m_beginY && m_beginY == m_endY)
            {
                //only substr
                if (x >= m_beginX)
                {
                    if (x <= m_endX)
                        x = m_beginX;
                    else
                        x -= m_endX - m_beginX + 1;
                }
            }
            else  if (y == m_beginY)
            {
                //first line
                if (x >= m_beginX)
                    x = m_beginX;
            }
            else if (y == m_endY)
            {
                //last line
                if (x <= m_endX)
                    x = m_beginX;
                else
                    x = m_beginX + x - m_endX - 1;
            }

            if (y <= m_endY)
                y = m_beginY;
            else
                y -= m_endY - m_beginY;
        }
        else if (m_selectType == select_t::line)
        {
            //line
            if (y <= m_endY)
                y = m_beginY;
            else
                y -= m_endY - m_beginY + 1;
        }
        else
        {
            //column
            if (y <= m_endY && y >= m_beginY)
                if (x >= m_beginX)
                {
                    if (x <= m_endX)
                        x = m_beginX;
                    else
                        x -= m_endX - m_beginX + 1;
                }
        }
    }

    int cx = static_cast<int>(x - m_xOffset);
    if (cx < 0 || cx >= m_clientSizeX)
        cx = -1;

    int cy = static_cast<int>(y - m_firstLine);
    if (cy < 0 || cy >= m_clientSizeY)
        cy = -1;

    if (cx < 0 || cy < 0)
        rc = _GotoXY(x, y);
    else
    {
        m_cursorx = static_cast<pos_t>(cx);
        m_cursory = static_cast<pos_t>(cy);
    }

    //del mark
    ChangeSelected(select_change::clear);
    InvalidateRect();

    return rc;
}

bool EditorWnd::UpdateLexPair()
{
    if (m_diff)
        return true;

    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;
    if (m_editor->CheckLexPair(y, x))
    {
        if (x >= m_xOffset   && x < m_xOffset + m_clientSizeX
         && y >= m_firstLine && y < m_firstLine + m_clientSizeY)
        {
            //if visible
            m_lexX = static_cast<int>(x);
            m_lexY = static_cast<int>(y);
            
            size_t bx, ex;
            select_line type;
            bool sel = GetSelectedPos(y, bx, ex, type);
            if(sel && bx <= x && x <= ex)
                Mark(x, y, x, y, ColorWindowSelectLMatch);
            else
                Mark(x, y, x, y, ColorWindowLMatch);
        }
        else
        {
            m_lexX = -1;
            m_lexY = -1;
        }
    }
    else
    {
        m_lexX = -1;
        m_lexY = -1;
    }

    return true;
}

bool EditorWnd::GetWord(std::u16string& str)
{
    size_t y = m_firstLine + m_cursory;
    auto curStr = m_editor->GetStr(y);
    if (curStr.empty())
        return false;

    str.clear();
    if (m_selectState == select_state::complete && m_selectType != select_t::line)
    {
        CorrectSelection();
        if (y == m_beginY && y == m_endY && m_endX - m_beginX >= 1)
        {
            for (size_t i = m_beginX; i <= m_endX; ++i)
            {
                auto c = curStr[i];
                str += c != S_TAB ? c : ' ';
            }

            return true;
        }
    }

    size_t b, e;
    if (!FindWord(curStr, b, e))
        return false;

    if (e - b < 1)
        return false;

    for (size_t i = b; i <= e; ++i)
    {
        auto c = curStr[i];
        str += c != S_TAB ? c : ' ';
    }

    return true;
}

bool EditorWnd::UpdateProgress(size_t step)
{
    if (m_progress != step)
    {
        std::stringstream sstr;
        sstr << "[" << std::setw(2) << step << "]";
        
        pos_t x{};
        if (m_border & BORDER_TOP)
            x += 2;

        WriteWnd(x, 0, sstr.str(), ColorWindowInfo);
        EditorApp::ShowProgressBar(step);

        WndManager::getInstance().Flush();

        m_progress = step;
        return CheckInput(1ms);
    }

    return false;
}

bool EditorWnd::IsWord(const std::u16string& str, size_t offset, size_t len)
{
    if ((offset == 0 || GetSymbolType(str[offset - 1]) != symbol_t::alnum)
     && (offset + len <= str.size() || GetSymbolType(str[offset + len]) != symbol_t::alnum))
        return true;
    else
        return false;
}

bool EditorWnd::FindUp(bool silence)
{
    if (FindDialog::s_vars.findStrW.empty())
    {
        EditorApp::SetErrorLine("Nothing to find");
        return false;
    }

    if (!silence)
        EditorApp::SetHelpLine("Search. Press any key for cancel");

    time_t t{ time(NULL) };

    std::u16string find{ FindDialog::s_vars.findStrW };
    size_t size = find.size();
    if (!FindDialog::s_vars.checkCase)
    {
        std::transform(find.begin(), find.end(), find.begin(),
            [](char16_t c) { return std::towupper(c); }
        );
    }

    //find only latin symbols
    bool fast{ true };
    for (auto c : find)
        if (c >= 0x80)
        {
            fast = false;
            break;
        }

    //search diaps
    size_t line{ m_firstLine + m_cursory };
    size_t end{};
    if (FindDialog::s_vars.inSelected && m_selectState == select_state::complete)
    {
        if (m_beginY <= m_endY)
        {
            if (line > m_endY)
                line = m_endY;
            end = m_beginY;
        }
        else
        {
            if (line > m_beginY)
                line = m_beginY;
            end = m_endY;
        }
    }

    size_t offset{ m_xOffset + m_cursorx };
    if (offset == 0)
    {
        if (line)
            --line;
        else
        {
            EditorApp::SetErrorLine("Nothing to find");
            return false;
        }
    }

    m_editor->FlushCurStr();

    size_t begin{ line };
    size_t progress{};
    bool userBreak{};
    while (line >= end)
    {
        auto str = m_editor->GetStrForFind(line, FindDialog::s_vars.checkCase, fast && offset == 0);
        if (0 != offset && offset > str.size())
        {
            offset = str.size();
        }

        auto itBegin = offset ? std::next(str.rbegin(), str.size() - offset) : str.rbegin();
        auto itFound = std::search(itBegin, str.rend(), std::boyer_moore_horspool_searcher(find.rbegin(), find.rend())); 
        if (itFound != str.rend())
        {
            if (fast && offset == 0)
            {
                str = m_editor->GetStrForFind(line, FindDialog::s_vars.checkCase, false);
                itFound = std::search(str.rbegin(), str.rend(), std::boyer_moore_horspool_searcher(find.rbegin(), find.rend()));
            }

            offset = std::distance(itFound, str.rend()) - size;
            if (!FindDialog::s_vars.findWord || IsWord(str, offset, size))
            {
                LOG_IF(time(NULL) - t, DEBUG) << "    Found time=" << time(NULL) - t;
                _GotoXY(offset, line);

                m_foundX = offset;
                m_foundY = line;
                if (offset - m_xOffset + size < static_cast<size_t>(m_clientSizeX))
                    m_foundSize = size;
                else
                    m_foundSize = m_clientSizeX - (offset - m_xOffset);

                Invalidate(m_foundY, invalidate_t::find, m_foundX, m_foundSize);

                if (!silence)
                    EditorApp::SetHelpLine();
                return true;
            }
        }
        else
            offset = 0;

        if (line)
            --line;
        else
            break;

        if (++progress == 1000)
        {
            progress = 0;
            if (userBreak = UpdateProgress((begin - line) * 99 / begin); userBreak)
                break;
        }
    }

    if (!silence)
    {
        HideFound();
        if (!userBreak)
            EditorApp::SetErrorLine("String not found");
        else
            EditorApp::SetHelpLine("User abort", stat_color::grayed);
    }

    return false;
}

bool EditorWnd::FindDown(bool silence)
{
    if (FindDialog::s_vars.findStrW.empty())
    {
        EditorApp::SetErrorLine("Nothing to find");
        return false;
    }

    if (!silence)
        EditorApp::SetHelpLine("Search. Press any key for cancel");

    time_t t{ time(NULL) };

    std::u16string find{ FindDialog::s_vars.findStrW };
    size_t size = find.size();
    if (!FindDialog::s_vars.checkCase)
    {
        std::transform(find.begin(), find.end(), find.begin(),
            [](char16_t c) { return std::towupper(c); }
        );
    }
    
    //find only latin symbols
    bool fast{true};
    for(auto c : find)
        if (c >= 0x80)
        {
            fast = false;
            break;
        }

    //search diaps
    size_t line{ m_firstLine + m_cursory };
    size_t end{m_editor->GetStrCount()};
    if (FindDialog::s_vars.inSelected && m_selectState == select_state::complete)
    {
        if (m_beginY <= m_endY)
        {
            if (line < m_beginY)
                line = m_beginY;
            end = m_endY;
        }
        else
        {
            if (line < m_endY)
                line = m_endY;
            end = m_beginY;
        }
    }

    size_t offset{ m_xOffset + m_cursorx };
    if (m_foundY == line && m_foundX == offset)
        offset += size;
    else
        ++offset;

    m_editor->FlushCurStr();

    size_t begin{ line };
    size_t progress{};
    bool userBreak{};
    while (line < end)
    {
        auto str = m_editor->GetStrForFind(line, FindDialog::s_vars.checkCase, fast && offset == 0);
        if (0 != offset && offset > str.size())
        {
            offset = 0;
            ++line;
            continue;
        }

        auto itBegin = offset ? std::next(str.begin(), offset) : str.begin();
        auto itFound = std::search(itBegin, str.end(), std::boyer_moore_horspool_searcher(find.begin(), find.end()));
        if(itFound != str.end())
        {
            if (fast && offset == 0)
            {
                str = m_editor->GetStrForFind(line, FindDialog::s_vars.checkCase, false);
                itFound = std::search(str.begin(), str.end(), std::boyer_moore_horspool_searcher(find.begin(), find.end()));
            }

            offset = std::distance(str.begin(), itFound);
            if (!FindDialog::s_vars.findWord || IsWord(str, offset, size))
            {
                LOG_IF(time(NULL) - t, DEBUG) << "    Found time=" << time(NULL) - t;
                _GotoXY(offset, line);

                m_foundX = offset;
                m_foundY = line;
                if (offset - m_xOffset + size < static_cast<size_t>(m_clientSizeX))
                    m_foundSize = size;
                else
                    m_foundSize = m_clientSizeX - (offset - m_xOffset);

                Invalidate(m_foundY, invalidate_t::find, m_foundX, m_foundSize);

                if (!silence)
                    EditorApp::SetHelpLine();
                return true;
            }
        }
        else
            offset = 0;

        ++line;

        if (++progress == 1000)
        {
            progress = 0;
            if (userBreak = UpdateProgress((line - begin) * 99 / (end - begin)); userBreak)
                break;
        }
    }

    if (!silence)
    {
        HideFound();
        if (!userBreak)
            EditorApp::SetErrorLine("String not found");
        else
            EditorApp::SetHelpLine("User abort", stat_color::grayed);
    }

    return false;
}

bool EditorWnd::EditWndCopy(EditorWnd* from)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditWndCopy";

    std::vector<std::u16string> buff;
    select_t type;

    bool rc = from->CopySelected(buff, type)
    && PasteSelected(buff, type);

    return rc;
}

bool EditorWnd::EditWndMove(EditorWnd* from)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditWndMove";

    std::vector<std::u16string> buff;
    select_t type;

    bool rc = from->CopySelected(buff, type)
        && PasteSelected(buff, type)
        && from->DelSelected();

    return rc;
}

Wnd* EditorWnd::CloneWnd()
{
    //LOG(DEBUG) << "CloneWnd";

    m_clonedWnd = std::make_shared<EditorWnd>();
    EditorWnd* wnd = m_clonedWnd.get();
    if (!wnd)
    {
        _assert(0);
        return nullptr;
    }

    wnd->SetEditor(m_editor);
    wnd->m_deleted = m_deleted;
    wnd->m_saved = m_saved;
    wnd->m_untitled = m_untitled;
    wnd->m_readOnly = m_readOnly;
    wnd->m_log = m_log;
    wnd->m_checkTime = m_checkTime;

    wnd->m_clone = true;
    wnd->m_visible = true;
    wnd->_GotoXY(m_xOffset + m_cursorx, m_firstLine + m_cursory);

    return wnd;
}

bool EditorWnd::Destroy() 
{ 
    if (m_clone)
    {
        auto list = m_editor->GetLinkedWnd(this);
        m_editor->UnlinkWnd(this);
        for (auto wnd : list)
        {
            if (!wnd->IsClone())
            {
                auto editorWnd = reinterpret_cast<EditorWnd*>(wnd);
                editorWnd->m_clonedWnd = nullptr;
                break;
            }
        }
    }
    return true; 
}

bool EditorWnd::GetSelectedLines(size_t& begin, size_t& end)
{
    if (!IsMarked())
        return false;
    if (m_beginY >= m_endY)
    {
        begin = m_beginY;
        end = m_endY;
    }
    else
    {
        begin = m_endY;
        end = m_beginY;
    }

    return true;
}

bool EditorWnd::ReplaceSubstr(size_t line, size_t pos, size_t len, const std::u16string& substr)
{
    if (m_readOnly)
        return true;

    size_t newSize = substr.size();
    bool rc = m_editor->ReplaceSubstr(true, line, pos, len, substr);
    if (len > newSize)
        ChangeSelected(select_change::delete_ch, line, pos, len - newSize);
    else if (newSize > len)
        ChangeSelected(select_change::insert_ch, line, pos, newSize - len);

    return rc;
}

bool EditorWnd::TryDeleteSelectedBlock()
{
    if (m_selectState != select_state::complete)
    {
        return false;
    }

    CorrectSelection();
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;
    if ((y < m_beginY || y > m_endY)
        || (m_selectType == select_t::stream && ((y == m_beginY && x < m_beginX) || (y == m_endY && x > m_endX)))
        || (m_selectType == select_t::column && (x < m_beginX || x > m_endX)))
    {
        //cursor out of selected block
        SelectUnselect(0);
        return false;
    }

    return EditBlockDel(0);
}

bool EditorWnd::CheckFileChanging() try
{
    bool rc{true};
    if (!m_untitled && m_checkTime <= std::chrono::system_clock::now())
    {
        //LOG(DEBUG) << "CheckFileChanging";
        m_checkTime = std::chrono::system_clock::now() + std::chrono::seconds(m_log ? LogFileCheckInterval : FileCheckInterval);

        auto state = m_editor->CheckFile();
        if (state == file_state::removed)
        {
            //file was deleted
            if (!m_deleted)
            {
                //lets try to wait one more 
                //as some programms can to recreate file while updated it
                m_deleted = true;
                LOG(DEBUG) << "Check deleted";
                return true;
            }
            else
            {
                //realy deleted
                LOG(DEBUG) << "Deleted";
                //check is all file in memory
                if (!m_editor->IsFileInMemory())
                {
                    _assert(0);
                    return true;
                }
                //ask for rewrite
                auto ret = MsgBox(MBoxKey::OK_CANCEL, "Restore",
                    { "File has been deleted outside of editor.",
                    "Do you want to restore it ?" },
                    { "Restore", "No" }
                );
                if (ret == ID_OK)
                {
                    //recreate file
                    std::ofstream ofs(m_editor->GetFilePath());
                    ofs.close();
                    //force save
                    rc = Save(1);
                }
            }
        }
        else if (state == file_state::changed)
        {
            //LOG(DEBUG) << "Changed";
            if (m_log)
            {
                size_t prevLines = m_editor->GetStrCount();
                rc = m_editor->LoadTail();
                size_t newLines = m_editor->GetStrCount();
                
                auto linked = m_editor->GetLinkedWnd();
                for (auto& wnd : linked)
                {
                    auto editorWnd = reinterpret_cast<EditorWnd*>(wnd);
                    size_t line{ editorWnd->m_firstLine + editorWnd->m_cursory };
                    bool last{ line >= prevLines };
                    if (last)
                    {
                        auto step = newLines - prevLines;
                        if (newLines < prevLines || line > prevLines
                            || step >= std::numeric_limits<uint16_t>::max())
                            editorWnd->MoveFileEnd(1);
                        else if (editorWnd->m_cursory < editorWnd->m_clientSizeY - static_cast<uint16_t>(step))
                            editorWnd->m_cursory += static_cast<uint16_t>(step);
                        else
                            editorWnd->MoveDown(static_cast<uint16_t>(step));
                    }
                    editorWnd->InvalidateRect(0, 0, m_clientSizeX, m_clientSizeY);
                    editorWnd->Repaint();
                }
            }
            else
            {
                //ask for reload
                auto ret = MsgBox(MBoxKey::OK_CANCEL, "Reload",
                    { "File has been modified outside of editor.",
                    "Do you want to reload it ?" },
                    { "Reload", "No" }
                );
                if (ret == ID_OK)
                    rc = Reload(0);
                m_checkTime = std::chrono::system_clock::now() + std::chrono::seconds(FileCheckInterval);
            }
        }

        m_deleted = false;
    }

    return rc;
}
catch (const std::exception& ex)
{
    LOG(ERROR) << __FUNC__ << "exception:" << ex.what();
    return false;
}

} //namespace _Editor
