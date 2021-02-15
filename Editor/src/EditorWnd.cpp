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
#include "EditorWnd.h"
#include "WndManager.h"

#include <algorithm>


bool EditorWnd::SetFileName(const std::filesystem::path& file, bool untitled, const std::string& parseMode)
{
    LOG(DEBUG) << "    SetFileName '" << file << "'";

    m_untitled = untitled;
    auto editor = std::make_shared<Editor>(file, parseMode);
    bool rc = editor->Load();
    if (!rc)
        return rc;
    return SetEditor(editor);
}

bool EditorWnd::SetEditor(EditorPtr editor) 
{ 
    LOG(DEBUG) << "    SetTextBuff";
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
    m_sizeX = 0;
    m_sizeY = 0;

    m_selectState = 0;
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
    LOG(DEBUG) << "    WndEdit::Refresh " << this;

    if (!WndManager::getInstance().IsVisible(this))
        return true;

    m_sizeX = GetCSizeX();
    m_sizeY = GetCSizeY();
    LOG(DEBUG) << "sx=" << m_sizeX << " sy=" << m_sizeY << " cx=" << m_cursorx << " cy=" << m_cursory;

    if (m_sizeX <= m_cursorx || m_sizeY <= m_cursory)
        _GotoXY(m_xOffset + m_cursorx, m_firstLine + m_cursory);

    bool rc = DrawBorder();

    if (!m_editor)
    {
        rc = Clr();
        return rc;
    }

    UpdateNameInfo();

    InvalidateRect(0, 0, m_sizeX, m_sizeY);
    Repaint();

    UpdateLexPair();

    return rc;
}

bool EditorWnd::UpdateAccessInfo()
{
    std::stringstream stream;
    stream << "[ " << GetAccessInfo() << "]";
    
    pos_t x = 0;
    if (m_border & BORDER_TOP)
        x += 2;

    WriteWnd(x, 0, stream.str(), ColorWindowInfo);
    return true;
}

bool EditorWnd::UpdateNameInfo()
{
    if (m_sizeX / 2 - 2 > 5)
    {
        pos_t x = 5;
        if (m_border & BORDER_TOP)
            x += 2;

        auto path = m_editor->GetFilePath().filename().u8string();
        path.resize(m_sizeX / 2 - 2, ' ');
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
    if (m_sizeX > 40)
        stream << "               [" << m_editor->GetStrCount() << "] L:" << y << " C:" << x;
    else
        stream << "               L:" << y << " C:" << x;

    auto str = stream.str();

    size_t len = std::max(str.size(), m_infoStrSize);
    len = std::min({len, static_cast<size_t>(32), static_cast<size_t>(m_sizeX / 2 + 3)});

    m_infoStrSize = str.size() - 14;

    WriteWnd(m_sizeX - (pos_t)len, 0, str.substr(str.size() - len), *m_pColorWindowTitle);
    return true;
}

bool EditorWnd::UpdateProgress(size_t d)
{
    return true;
}

bool EditorWnd::UpdateLexPair()
{
    //???
    return true;
}

bool EditorWnd::_GotoXY(size_t x, size_t y, bool top)
{
    if (m_sizeX == 0 || m_sizeY == 0)
    {
        m_sizeX = GetCSizeX();
        m_sizeY = GetCSizeY();
    }

    size_t line = 0;
    size_t pos = 0;

    if (y > m_editor->GetStrCount())
        y = m_editor->GetStrCount();

    if (x == 0)
        m_cursorx = 0;
    else if (x < m_sizeX - 1)
        m_cursorx = static_cast<pos_t>(x);
    else
    {
        m_cursorx = m_sizeX - 5;//we will see 4 symbols
        pos = x - m_cursorx;
        if (pos >= MAX_STRLEN - m_sizeX)
        {
            pos = MAX_STRLEN - m_sizeX;
            m_cursorx = static_cast<pos_t>(x - pos);
        }
    }

    if (!top)
    {
        if (y <= 0)
            m_cursory = 0;
        else if (y >= m_firstLine && y < m_firstLine + m_sizeY)
        {
            line = m_firstLine;
            m_cursory = static_cast<pos_t>(y - line);
        }
        else if (y < m_sizeY / 2)
            m_cursory = static_cast<pos_t>(y);
        else
        {
            m_cursory = m_sizeY / 2;
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
        rc = InvalidateRect(0, 0, m_sizeX, m_sizeY);
    }

    return rc;
}

bool EditorWnd::InvalidateRect(pos_t x, pos_t y, pos_t sizex, pos_t sizey)
{
    if (0 == m_sizeX || 0 == m_sizeY)
    {
        m_sizeX = GetCSizeX();
        m_sizeY = GetCSizeY();
    }

    if (sizex == 0)
        sizex = m_sizeX;
    if (sizey == 0)
        sizey = m_sizeY;

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
        size = MAX_STRLEN;

    int y = static_cast<int>(line) - static_cast<int>(m_firstLine);
    int x = static_cast<int>(pos) - static_cast<int>(m_xOffset);

    int endx = x + static_cast<int>(size);
    if (endx >= m_sizeX)
        endx = m_sizeX;

    if (x < 0 && endx > 0)
        x = 0;

    LOG(DEBUG) << "    Invalidate line=" << line << " type=" << static_cast<int>(type) << " pos=" << pos << " size=" << size << "; x=" << x << " y=" << y << " endx=" << endx;

    if (y < 0 && type != invalidate_t::find && type != invalidate_t::change)
    {
        y = 0;
        type = invalidate_t::full;
    }

    if (y < 0 || y >= m_sizeY || x < 0 || x >= m_sizeX || endx < 0)
        return true;

    switch (type)
    {
    case invalidate_t::find:
    case invalidate_t::change:
        InvalidateRect(static_cast<pos_t>(x), static_cast<pos_t>(y), static_cast<pos_t>(endx - x), 1);
        break;
    case invalidate_t::del:
        InvalidateRect(0, static_cast<pos_t>(y), m_sizeX, static_cast<pos_t>(m_sizeY - y));
        break;
    case invalidate_t::insert:
        InvalidateRect(0, static_cast<pos_t>(y), m_sizeX, static_cast<pos_t>(m_sizeY - y));
        break;
    case invalidate_t::full:
        InvalidateRect(0, 0, m_sizeX, m_sizeY);
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
        if (m_invBeginX > m_sizeX)
            m_invBeginX = 0;
        if (m_invEndX > m_sizeX)
            m_invEndX = m_sizeX;

        if (m_invBeginY > m_sizeY)
            m_invBeginY = 0;
        if (m_invEndY > m_sizeY)
            m_invEndY = m_sizeY;

        StopPaint();

        for (pos_t i = m_invBeginY; i < m_invEndY; ++i)
        {
            auto str = m_editor->GetStr(m_firstLine + i);
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
    rc = ShowBuff(0, static_cast<pos_t>(-1), m_sizeX, 1);

    return rc;
}

bool EditorWnd::IsNormalSelection(size_t bx, size_t by, size_t ex, size_t ey)
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

bool EditorWnd::PrintStr(pos_t x, pos_t y, const std::u16string& str, size_t offset, size_t len)
{
    auto MarkFound = [this, y]() {
        if (m_foundSize && y == m_foundY)
            //mark found
            Mark(m_foundX, m_foundY, m_foundX + m_foundSize - 1, m_foundY, ColorWindowFound);
    };

    //if(!m_pDiff || !m_pDiff->IsDiff(m_nDiffBuff, m_nFirstLine + y))
//    if (!m_diff) //???
    {
        std::vector<color_t> colorBuff;
        colorBuff.reserve(MAX_STRLEN);
        bool rc = m_editor->GetColor(m_firstLine + y, str, colorBuff, offset + len);

        if (rc)
            rc = WriteColorStr(x, y, str.substr(offset, len), std::vector<color_t>(colorBuff.cbegin() + offset, colorBuff.cend()));
        else
            rc = WriteWStr(x, y, str.substr(offset, len));
    }
/*
    else
    {
        if (m_pDiff->IsDiff(m_nDiffBuff, m_nFirstLine + y))
            SetTextAttr(ColorWindowDiff);
        else
            SetTextAttr(ColorWindowNotDiff);
        rc = WriteWStr(x, y, pStr + offset);
    }
*/

    if (!m_selectState || (m_selectState & 0xff00) == 0)
    {
        //not selected
        MarkFound();
        return true;
    }

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

    size_t posy = y + m_firstLine;

    if (posy < y1 || posy > y2)
    {
        //not selected
        MarkFound();
        return true;
    }

    size_t bx, ex;
    if (m_selectType == select_t::stream)
    {
        //stream
        if (posy == y1)
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
                ex = MAX_STRLEN;
            }
        }
        else if (posy == y2)
        {
            //last line
            //fill 0-x2
            bx = 0;
            ex = x2;
        }
        else
        {
            //fill 0-.
            bx = 0;
            ex = MAX_STRLEN;
        }
    }
    else if (m_selectType == select_t::line)
    {
        //line
        //fill 0-.
        bx = 0;
        ex = MAX_STRLEN;
    }
    else
    {
        //columns
        //fill x1-x2
        bx = x1;
        ex = x2;
    }

    bool rc;

//    if (!m_diff)
        rc = Mark(bx, posy, ex, posy, ColorWindowSelect, m_selectType);
//    else
//        rc = Mark(bx, posy, ex, posy, ColorWindowCurDiff, m_selectType);

    MarkFound();

    return rc;
}

bool EditorWnd::Mark(size_t bx, size_t by, size_t ex, size_t ey, color_t color, select_t selectType)
{
    LOG(DEBUG) << "    Mark bx=" << bx << " by=" << by << " ex=" << ex << " ey=" << ey << " color=" << color << " select=" << static_cast<int>(selectType);

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

        if (y2 < m_firstLine || y1 >= m_firstLine + m_sizeY)
            //not visible
            return true;

        if (y1 < m_firstLine)
        {
            y1 = m_firstLine;
            x1 = 0;
        }
        if (y2 >= m_firstLine + m_sizeY)
        {
            y2 = m_firstLine + m_sizeY - 1;
            x2 = MAX_STRLEN;
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

        if (y2 < m_firstLine || y1 >= m_firstLine + m_sizeY)
            //not visible
            return true;

        if (y1 < m_firstLine)
            y1 = m_firstLine;

        if (y2 >= m_firstLine + m_sizeY)
            y2 = m_firstLine + m_sizeY - 1;
    }

    LOG(DEBUG) << "    Mark1 bx=" << x1 << " by=" << y1 << " ex=" << x2 << " ey=" << y2;

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
                    ex = MAX_STRLEN;
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
                ex = MAX_STRLEN;
            }
        }
        else if (selectType == select_t::line)
        {
            //line
            //fill 0-.
            bx = 0;
            ex = MAX_STRLEN;
        }
        else
        {
            //column
            //fill x1-x2
            bx = x1;
            ex = x2;
        }

        if (ex < m_xOffset || bx >= m_xOffset + m_sizeX)
            //not visible
            continue;
        if (bx < m_xOffset)
            bx = m_xOffset;
        if (ex >= m_xOffset + m_sizeX)
            ex = m_xOffset + m_sizeX - 1;

        if (color)
            ColorRect(static_cast<pos_t>(bx - m_xOffset), static_cast<pos_t>(y - m_firstLine), static_cast<pos_t>(ex - bx + 1), 1, color);
        else
        {
            auto str = m_editor->GetStr(y);
            std::vector<color_t> colorBuff;
            colorBuff.reserve(MAX_STRLEN);
            bool rc = m_editor->GetColor(y, str, colorBuff, ex + 1);
            if (rc)
            {
                LOG(DEBUG) << "WriteColor bx=" << bx << " ex=" << ex << " y=" << y;
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
    LOG(DEBUG) << "    WndEdit:WndProc " << std::hex << code << std::dec;
/* //???
    if (code == K_TIME)
    {
        if (!m_Timer)
        {
            m_Timer = 5 * 2;//2 sec
            int rc = m_pTBuff->CheckAccess();
            if (rc)
            {
                int mode = m_pTBuff->GetAccessInfo();
                long long size = m_pTBuff->GetSize();

                TPRINT(("File was changed mode=%c size=%lld\n", mode, size));
                if (mode == 'N' || !size)
                {
                    //файл удален
                    if (!m_fDeleted)
                    {
                        //еще одно ожидание
                        //многие программы сначала удал€ют файл
                        //а потом восстанавливают
                        TPRINT(("Check deleted\n"));
                        m_fDeleted = 1;
                    }
                    else
                    {
                        m_fDeleted = 0;
                        //нужно проверить изменен ли файл
                        //если изменен, весь ли он в пам€ти
                        FLoadDialog Dlg(GetObjPath(), 1);
                        int code1 = Dlg.Activate();
                        if (code1 == ID_OK)
                        {
                            //TPRINT(("...Save1\n"));
                            Save(1);
                        }
                    }
                }
                else
                {
                    m_fDeleted = 0;
                    if (!m_fLog)
                    {
                        FLoadDialog Dlg(GetObjPath());
                        int code1 = Dlg.Activate();
                        if (code1 == ID_OK)
                        {
                            //TPRINT(("...Reload1\n"));
                            Reload(0);
                        }
                    }
                    else
                    {
                        //???
                        //TPRINT(("...Reload2\n"));
                        Reload(0);
                    }
                }
            }
            else if (m_fDeleted)
            {
                m_fDeleted = 0;
                //нужно проверить изменен ли файл
                //если изменен, весь ли он в пам€ти
                FLoadDialog Dlg(GetObjPath(), 1);
                int code1 = Dlg.Activate();
                if (code1 == ID_OK)
                {
                    //TPRINT(("...Save2\n"));
                    Save(1);
                }
            }
        }
        --m_Timer;
    }
*/

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

    if (code != K_TIME)
    {
        Repaint();

        UpdateLexPair();
        m_editor->SetUndoRemark("");

        //refresh all view
        m_editor->RefreshAllWnd(this);
    }

    if (m_close)
        Wnd::Destroy();

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
        if ((cmd & K_TYPEMASK) == K_MOUSEKUP)
        {
            m_selectMouse = false;
            InputRelease();
            MovePos(cmd);
            SelectEnd(cmd);

            if (m_popupMenu)
            {
                m_popupMenu = true;
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
                    if ((cmd & K_CTRL) && m_selectState == 1)
                        m_selectType = select_t::line;
                    else if ((cmd & K_SHIFT) && m_selectState == 1)
                        m_selectType = select_t::column;
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
        case K_ENTER:
            EditEnter(cmd);
            break;
        case K_TAB:
            EditTab(cmd);
            break;
        case K_BS:
            EditBS(cmd);
            break;
        default:
            EditC(cmd);
            break;
        }
    }
    else
    {
        EditorCmd ecmd = static_cast<EditorCmd>((cmd - EDITOR_CMD) >> 16);

        if (m_selectMouse)
        {
            LOG(DEBUG) << "Mouse select for " << ecmd;
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
            LOG(ERROR) << "    ??? editor cmd=" << std::hex << cmd << std::dec;
        }
        else
        {
            PutMacro(cmd);

            auto& [func, select] = it->second;

            if (select == Select::end)
            {
                SelectEnd(cmd);
            }

            [[maybe_unused]]bool rc = func(this, cmd);

            if (select == Select::begin
                && (m_selectState & 0xff) == 1)
            {
                SelectBegin(cmd);
            }
        }
    }

    return 0;
}

bool EditorWnd::HideFound()
{
    bool rc;
    if (m_lexX >= 0 && m_lexY >= 0)
    {
        LOG(DEBUG) << "    HideLex x=" << m_lexX << " y=" << m_lexY;
        if ( m_lexY >= m_firstLine && m_lexY < m_firstLine + m_sizeY
          && m_lexX >= m_xOffset   && m_lexX < m_xOffset + m_sizeX)
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
        //TPRINT(("    HideFound x=%d y=%d s=%d\n", m_nFoundX, m_nFoundY, m_nFoundSize));
        size_t size = m_foundSize;
        m_foundSize = 0;

        if (m_foundY >= m_firstLine && m_foundY < m_firstLine + m_sizeY)
        {
            auto str = m_editor->GetStr(m_foundY);

            int x = static_cast<int>(m_foundX) - static_cast<int>(m_xOffset);
            if (x < 0)
            {
                size += x;
                x = 0;
            }

            if (x + size > m_sizeX)
            {
                size -= x + size - m_sizeX;
            }

            if (size > 0)
                rc = PrintStr(static_cast<pos_t>(x), static_cast<pos_t>(m_foundY - m_firstLine), str, x + m_xOffset, size);
        }
    }
    return 0;
}
