/*
FreeBSD License

Copyright (c) 2020 vikonix: valeriy.kovalev.software@gmail.com
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
#include "WndManager.h"
#include "App.h"

static const pos_t SPLIT_WIDTH { 1 };


bool WndManager::Init()
{
    bool rc = m_console.Init();
    if (!rc)
        return false;

    m_console.GetScreenSize(m_sizex, m_sizey);

    LOG(INFO) << __FUNCTION__ << " x=" << m_sizex << " y=" << m_sizey;
    CalcView();

    m_textBuff.SetSize(m_sizex, m_sizey);

    return true;
}

bool WndManager::Deinit()
{
    LOG(INFO) << __FUNCTION__;

    if (m_view[2].wnd)
        m_view[2].wnd = nullptr;

    m_wndList.clear();

    bool rc = true;
    if (0 != m_textBuff.GetSize())
    {
        rc = m_console.SetTextAttr(DEFAULT_COLOR)
          && m_console.ClrScr();

        m_textBuff.SetSize();
    }

    m_console.Deinit();
    return rc;
}

bool WndManager::CalcView()
{
    LOG(DEBUG) << __FUNCTION__ << " t=" << static_cast<int>(m_splitType);

    //for dialogs
    m_view[0].left  = 0;
    m_view[0].top   = m_topLines;
    m_view[0].sizex = m_sizex;
    m_view[0].sizey = m_sizey - m_topLines - m_bottomLines;

    //for work windows
    if (m_splitType == split_t::split_h)
    {
        //horizontal
        m_view[1].left  = 0;
        m_view[1].top   = m_topLines;
        m_view[1].sizex = m_sizex;
        m_view[1].sizey = m_splitY;

        m_view[2].left  = 0;
        m_view[2].top   = m_topLines + m_splitY + 1;
        m_view[2].sizex = m_sizex;
        m_view[2].sizey = m_sizey - m_topLines - m_bottomLines - m_splitY - 1;
    }
    else if (m_splitType == split_t::split_v)
    {
        //vertical
        m_view[1].left  = 0;
        m_view[1].top   = m_topLines;
        m_view[1].sizex = m_splitX;
        m_view[1].sizey = m_sizey - m_topLines - m_bottomLines;

        m_view[2].left  = m_splitX + SPLIT_WIDTH;
        m_view[2].top   = m_topLines;
        m_view[2].sizex = m_sizex - m_splitX - SPLIT_WIDTH;
        m_view[2].sizey = m_sizey - m_topLines - m_bottomLines;
    }

    return true;
}

bool WndManager::Refresh()
{
    LOG(DEBUG) << "  M::Refresh";

    if (!m_sizex || !m_sizey)
        return false;

    m_disablePaint = 1;
    m_textBuff.Fill(0);

    bool rc;
    if (m_logo.fillChar == 0)
    {
        rc = SetTextAttr(ColorScreen)
          && Cls();
    }
    else
    {
        //with logo
        rc = FillRect(0, 0, m_sizex, m_sizey, m_logo.fillChar, m_logo.fillColor);

        pos_t x = m_logo.x;
        pos_t y = m_logo.y;

        if (x < 0)
            x = (m_sizex - (pos_t)m_logo.logoStr.front().size()) / 2;
        if (y < 0)
            y = (m_sizey - (pos_t)m_logo.logoStr.size()) / 2;

        SetTextAttr(m_logo.logoColor);

        for(const auto& str : m_logo.logoStr)
        {
            GotoXY(x, y++);
            WriteStr(str);
        }
    }

    rc = Application::getInstance().Repaint();
    
    if (!m_wndList.empty())
    {
        if (m_wndList.size() > 1 && m_wndList[0]->GetWndType() == wnd_t::dialog)
            m_wndList[1]->Refresh();

        if (m_view[2].wnd)
        {
            if (m_splitType == split_t::split_h)
                FillRect(m_view[2].left, m_view[2].top - 1, m_view[2].sizex, 1,
                    ' '/*ACS_HLINE*/, ColorViewSplitter);
            else if (m_splitType == split_t::split_v)
                FillRect(m_view[2].left - SPLIT_WIDTH, m_view[2].top, SPLIT_WIDTH, m_view[2].sizey,
                    ' '/*ACS_VLINE*/, ColorViewSplitter);

            m_view[2].wnd->Refresh();
        }

        rc = m_wndList[0]->Refresh();
    }

    m_disablePaint = 0;
    rc = ShowBuff();
    return rc;
}

bool WndManager::Cls()
{
    LOG(DEBUG) << "  M::Cls";

    HideCursor();
    bool rc = CallConsole(ClrScr());
    return rc;
}


bool WndManager::SetTextAttr(color_t color)
{
    //LOG(DEBUG) << __FUNCTION__ << " c=" << std::hex << color << std::dec;
    m_color = color;
    bool rc = CallConsole(SetTextAttr(color));
    return rc;
}

bool WndManager::GotoXY(pos_t x, pos_t y)
{
    m_cursorx = x;
    m_cursory = y;
    bool rc = CallConsole(GotoXY(x, y));
    return rc;
}

bool WndManager::ShowInputCursor(cursor_t cursor, pos_t x, pos_t y)
{
    if (cursor == cursor_t::CURSOR_HIDE || m_wndList.empty())
    {
        x = 0;
        y = 0;
        cursor = cursor_t::CURSOR_HIDE;
    }
    else if (x < 0 || y < 0)
    {
        if (!m_activeView)
        {
            x = m_wndList[0]->m_cursorx;
            y = m_wndList[0]->m_cursory;
            m_wndList[0]->ClientToScreen(x, y);
        }
        else
        {
            x = m_view[2].wnd->m_cursorx;
            y = m_view[2].wnd->m_cursory;
            m_view[2].wnd->ClientToScreen(x, y);
        }
    }

    bool rc = true;
    if (m_cursorx != x || m_cursory != y || m_cursor != cursor)
    {
        rc = m_console.GotoXY(x, y)
          && m_console.SetCursor(cursor)
          && m_console.Flush();

        m_cursorx = x;
        m_cursory = y;
        m_cursor  = cursor;
    }

    return rc;
}

bool WndManager::HideCursor()
{
    int rc = 0;
    if (m_cursor != cursor_t::CURSOR_HIDE)
    {
        rc = m_console.SetCursor(cursor_t::CURSOR_HIDE);
        m_cursor = cursor_t::CURSOR_HIDE;
    }
    return rc;
}

bool WndManager::FillRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, input_t c, color_t color)
{
    LOG(DEBUG) << "  M::FillRect l=" << left << " t=" << top << " sx=" << sizex << " sy=" << sizey 
        << " c=" << c << " color=" << std::hex << color << std::dec;

    HideCursor();
    cell_t cl = MAKE_CELL(0, color, c);
    for (pos_t y = 0; y < sizey; ++y)
    {
        for (pos_t x = 0; x < sizex; ++x)
            m_textBuff.SetSell((size_t)left + x, (size_t)top + y, cl);
    }

    bool rc = WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_textBuff);
    return rc;
}

bool WndManager::InvColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey)
{
    LOG(DEBUG) << "  M::InvColorRect l=" << left << " t=" << top << " sx=" << sizex << " sy=" << sizey;

    HideCursor();
    for (pos_t y = 0; y < sizey; ++y)
    {
        for (pos_t x = 0; x < sizex; ++x)
        {
            color_t color = COLOR_INVERSE(GET_CCOLOR(m_textBuff.GetCell((size_t)left + x, (size_t)top + y)));
            cell_t cl = MAKE_CELL(0, color, m_textBuff.GetCell((size_t)left + x, (size_t)top + y));
            m_textBuff.SetSell((size_t)left + x, (size_t)top + y, cl);
        }
    }

    int rc = WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_textBuff);
    return rc;
}

bool WndManager::ShowBuff()
{
    if (0 == m_textBuff.GetSize())
        return false;

    bool rc = WriteBlock(0, 0, m_sizex - 1, m_sizey - 1, m_textBuff);
    return rc;
}

bool WndManager::ShowBuff(pos_t left, pos_t top, pos_t sizex, pos_t sizey)
{
    if (0 == m_textBuff.GetSize())
        return false;

    bool rc = false;
    if (left < 0 || top < 0 || left + sizex > m_sizex || top + sizey > m_sizey)
        LOG(ERROR) << "  M::ShowBuff l=" << left << " t=" << top << " sx=" << sizex << " sy=" << sizey;
    else
        rc = WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_textBuff);
    return rc;
}

bool WndManager::WriteBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, const ScreenBuffer& block)
{
    HideCursor();
    bool rc = CallConsole(WriteBlock(left, top, right, bottom, block, left, top));
    return rc;
}

bool WndManager::GetBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, std::vector<cell_t>& block)
{
    block.clear();
    for (pos_t x = left; x <= right; ++x)
        for (pos_t y = top; y <= bottom; ++y)
            block.push_back(m_textBuff.GetCell(x, y));
    return true;
}

bool WndManager::PutBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, const std::vector<cell_t>& block)
{
    if (block.empty())
        return true;

    HideCursor();
    size_t i = 0;
    for (pos_t x = left; x <= right; ++x)
        for (pos_t y = top; y <= bottom; ++y)
            m_textBuff.SetSell(x, y, block[i++]);

    bool rc = WriteBlock(left, top, right, bottom, m_textBuff);
    return rc;
}

bool WndManager::WriteStr(const std::string& str)
{
    HideCursor();

    std::u16string wstr = utf8::utf8to16(str);

    size_t l = wstr.size();

    bool rc;
    if (m_cursory != m_sizey - 1 || m_cursorx + (pos_t)l <= m_sizex - 1)
        rc = CallConsole(WriteStr(wstr));
    else
    {
        char16_t PrevC = wstr[l - 2];
        char16_t LastC = wstr[l - 1];
        rc = CallConsole(WriteStr(wstr.substr(0, l - 1)));
        rc = CallConsole(WriteLastChar(PrevC, LastC));
    }

    for(const auto& c : wstr)
        m_textBuff.SetSell(m_cursorx++, m_cursory, MAKE_CELL(0, m_color, c));

    return rc;
}

bool WndManager::Resize(pos_t sizex, pos_t sizey)
{
    LOG(INFO) << "  M::Resize x=" << sizex << " y=" << sizey;

    m_sizex = sizex;
    m_sizey = sizey;
    CalcView();

    m_textBuff.SetSize(sizex, sizey);

    bool rc = Refresh();
    return rc;
}

bool WndManager::CheckRefresh()
{
    if (m_invalidate)
    {
        m_invalidate = 0;
        WriteConsoleTitle();
        Refresh();
    }
    return 0;
}

bool WndManager::WriteConsoleTitle(bool set)
{
    if (!set && !m_invalidTitle)
        return true;

    std::wstring name;
    if (!m_wndList.empty())
    {
        if (m_view[2].wnd && m_activeView == 1)
            name = m_view[2].wnd->GetObjName();
        else
            name = m_wndList[0]->GetObjName();
    }

    bool rc = m_console.WriteConsoleTitle(name + L" - " + Application::getInstance().m_appName);

    return rc;
}

bool WndManager::WriteChar(char c)
{
    std::string str { c };
    std::u16string wstr = utf8::utf8to16(str);
    return WriteWChar(wstr[0]);
}

bool WndManager::WriteWChar(char16_t c)
{
    //LOG(DEBUG) << __FUNCTION__ << std::hex << c << std::dec;
    HideCursor();
    bool rc = true;
    if (m_cursory != m_sizey - 1 || m_cursorx < m_sizex - 1)
        rc = CallConsole(WriteChar(c));
    else
    {
        char16_t PrevC = GET_CTEXT(m_textBuff.GetCell((size_t)m_cursorx - 1, m_cursory));
        rc = CallConsole(WriteLastChar(PrevC, c));
    }

    m_textBuff.SetSell(m_cursorx++, m_cursory, MAKE_CELL(0, m_color, c));

    return rc;
}

bool WndManager::ColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, color_t color)
{
    LOG(DEBUG) << "  M::ColorRect l=" << left << " t=" << top << " x=" << sizex << " y=" << sizey << " c=" << color;

    HideCursor();
    for (pos_t y = 0; y < sizey; ++y)
    {
        for (pos_t x = 0; x < sizex; ++x)
        {
            cell_t cl = MAKE_CELL(0, color, m_textBuff.GetCell((size_t)left + x, (size_t)top + y));
            m_textBuff.SetSell((size_t)left + x, (size_t)top + y, cl);
        }
    }

    int rc = WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_textBuff);
    return rc;
}

bool WndManager::Show(Wnd* wnd, bool refresh, int view)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd << " r=" << refresh << " v=" << view;

    if (!view)
    {
        m_activeView = 0;
        AddWnd(wnd);
    }
    else
        SetTopWnd(wnd, view);

    bool inv = m_invalidate;
    if (!inv && !refresh)
        m_invalidate = false;

    return true;
}

bool WndManager::Hide(Wnd* wnd, bool refresh)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd << " r=" << refresh;

    if (wnd == m_view[2].wnd)
    {
        m_view[2].wnd = nullptr;
        m_splitType = split_t::no_split;
        m_activeView = 0;
        m_invalidate = true;
        CalcView();
    }
    else
    {
        DelWnd(wnd);
        bool inv = m_invalidate;
        if (!inv && !refresh)
            m_invalidate = false;
    }

    return true;
}

const View& WndManager::GetView(const Wnd* wnd) const
{
    int view = 0;

    if (split_t::no_split != m_splitType && wnd->IsUsedView())
    {
        if (m_view[2].wnd == wnd)
            view = 2;
        else
            view = 1;
    }

    LOG(DEBUG) << __FUNCTION__ << " n=" << view;

    return m_view[view];
}

bool WndManager::CloneView(const Wnd* wnd)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd;

    if (m_view[2].wnd)
    {
        if (m_view[2].wnd->IsClone())
            m_view[2].wnd->Destroy();
        else
        {
            if (!m_activeView)
                AddLastWnd(m_view[2].wnd);
            else
                AddWnd(m_view[2].wnd);
        }
    }

    if (!wnd)
    {
        m_activeView = false;
        m_splitType = split_t::no_split;
        m_view[2].wnd = nullptr;
    }
    else
    {
        m_view[2].wnd = wnd->CloneWnd();
    }

    return 0;
}

bool WndManager::AddWnd(Wnd* wnd)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd;

    //add to top
    m_wndList.push_front(wnd);
    m_invalidate = true;
    m_invalidTitle = true;
    return true;
}

bool WndManager::AddLastWnd(Wnd* wnd)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd;
    //add to bottom
    if (m_wndList.empty())
    {
        m_invalidate = true;
        m_invalidTitle = true;
    }
    m_wndList.push_back(wnd);

    return true;
}

bool WndManager::DelWnd(Wnd* wnd)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd;

    //del from list
    if (wnd == m_view[2].wnd)
    {
        CloneView();
    }
    for (auto it = m_wndList.begin(); it != m_wndList.end(); ++it)
    {
        if (*it == wnd)
        {
            m_wndList.erase(it);
            break;
        }
    }
    m_invalidate = true;
    m_invalidTitle = true;

    return true;
}

size_t WndManager::GetWndCount() const
{
    return m_wndList.size() + m_view[2].wnd != nullptr ? 1 : 0;
}


Wnd* WndManager::GetWnd(int n, int view)
{
    if (view == -1)
        view = m_activeView;

    if (view == 1 || n < 0)
        return m_view[2].wnd;

    if (n < (int)m_wndList.size())
        return m_wndList[n];
    else
        return nullptr;
}

bool WndManager::SetTopWnd(int n, int view)
{
    LOG(DEBUG) << __FUNCTION__ << " n=" << n << " v=" <<view;

    if ((n == 0 && view == 0) || (n < 0 && view != 0))
        return true;

    if (n < 0)
        return SetTopWnd(m_view[2].wnd);
    else if (n < (int)m_wndList.size())
        return SetTopWnd(m_wndList[n], view);
    else
        return false;
}

bool WndManager::SetTopWnd(Wnd* wnd, int view)
{
    LOG(DEBUG) << __FUNCTION__ << " w=" << wnd << " view=" << view;
    LOG(DEBUG) << "top=" << m_wndList[0] << " v2=" << m_view[2].wnd << " av=" << m_activeView;

    if (!wnd)
        return true;

    if (split_t::no_split == m_splitType)
        view = 0;
    else if (view == -1)
        view = m_activeView;

    if (!view)
    {
        if (wnd == m_wndList[0])
            return true;

        if (m_view[2].wnd && m_view[2].wnd->IsClone())
        {
            Wnd* pDel = m_view[2].wnd;
            m_view[2].wnd = nullptr;
            pDel->Destroy();

            Wnd* pTop = m_wndList[0];
            DelWnd(pTop);
            m_view[2].wnd = pTop;
        }

        if (wnd != m_view[2].wnd)
            DelWnd(wnd);
        else
        {
            m_view[2].wnd = nullptr;
            CloneView(wnd);
        }
        AddWnd(wnd);
    }
    else
    {
        if (m_view[2].wnd == wnd)
            return true;

        if (m_view[2].wnd)
        {
            if (!m_view[2].wnd->IsClone())
            {
                AddLastWnd(m_view[2].wnd);
                m_view[2].wnd = nullptr;
            }
            else
            {
                Wnd* pDel = m_view[2].wnd;
                m_view[2].wnd = nullptr;
                pDel->Destroy();
            }
        }

        if (wnd == m_wndList[0])
            CloneView(wnd);
        else
        {
            DelWnd(wnd);
            m_view[2].wnd = wnd;
        }
    }

    return true;
}

bool WndManager::ProcInput(input_t code)
{
    LOG(DEBUG) << "  M:ProcInput " <<  std::hex << code << std::dec;
    bool rc = 0;

    if (code == K_REFRESH)
    {
        //refresh
        LOG(DEBUG) << "WndManager Refresh";
        Refresh();
    }
    else if ((code & K_TYPEMASK) == K_RESIZE)
        rc = Resize(K_GET_X(code), K_GET_Y(code));
    else
    {
        if (!m_wndList.empty())
        {
            if ((code & K_MOUSE) == K_MOUSE)
            {
                //mouse event
                if ((code & K_TYPEMASK) == K_MOUSEKUP
                    || (code & K_MOUSEW) == K_MOUSEW)
                {
                    if (!m_activeView)
                        rc = m_wndList[0]->EventProc(code);
                    else
                        rc = m_view[2].wnd->EventProc(code);
                }
                else
                {
                    //mouse click
                    pos_t x = K_GET_X(code);
                    pos_t y = K_GET_Y(code);

                    if (m_wndList[0]->CheckWndPos(x, y))
                    {
                        SetActiveView(0);
                        m_wndList[0]->ScreenToClient(x, y);
                        if (m_wndList[0]->CheckClientPos(x, y))
                            rc = m_wndList[0]->EventProc(code);
                    }
                    else if (m_view[2].wnd)
                    {
                        if (m_view[2].wnd->CheckWndPos(x, y))
                        {
                            SetActiveView(1);
                            m_view[2].wnd->ScreenToClient(x, y);
                            if (m_view[2].wnd->CheckClientPos(x, y))
                                rc = m_view[2].wnd->EventProc(code);
                        }
                        else
                        {
                            LOG(DEBUG) << "Split line";
                            if ((code & K_TYPEMASK) == K_MOUSEKL)
                                TrackView({});
                        }
                    }
                }
            }
            else
            {
                //other event
                if ((code & K_TYPEMASK) != K_TIME)
                {
                    if (!m_activeView)
                        rc = m_wndList[0]->EventProc(code);
                    else
                        rc = m_view[2].wnd->EventProc(code);
                }
                else if (m_wndList[0]->IsUsedTimer())
                {
                    for(auto& wnd : m_wndList)
                        wnd->EventProc(code);

                    if (m_view[2].wnd)
                        m_view[2].wnd->EventProc(code);
                }
            }
        }
    }

    return rc;
}

bool WndManager::SetActiveView(int n)
{
    LOG(DEBUG) << "SetActiveView st=" << static_cast<int>(m_splitType) << " av=" << m_activeView << " n=" << n;

    if (m_splitType == split_t::no_split)
        return 0;

    if (n < 0)
        m_activeView = m_activeView ? 0 : 1;
    else if (n <= 1)
        m_activeView = n;
    else
        m_activeView = 0;

    return WriteConsoleTitle(true);
}

bool WndManager::ChangeViewMode(int type)//0-create/del 1-horiz/vert
{
    LOG(DEBUG) << "ChangeViewMode st=" << static_cast<int>(m_splitType) << " t=" << type;

    if (m_wndList.empty())
        return true;

    if (!m_splitX || !m_splitY)
    {
        m_splitX = m_sizex / 2;
        m_splitY = m_sizey / 2;
    }

    if (type == 0)
    {
        //create/del
        if (m_splitType != split_t::no_split)
        {
            m_splitType = split_t::no_split;
            CloneView();
        }
        else
        {
            m_splitType = split_t::split_h;
            CalcView();
            CloneView(m_wndList[0]);
        }
    }
    else
    {
        if (m_splitType == split_t::no_split)
            return 0;

        if (m_splitType == split_t::split_h)
            m_splitType = split_t::split_v;
        else
            m_splitType = split_t::split_h;
    }

    bool rc = CalcView()
    && Refresh();

    return rc;
}


bool WndManager::SetView(pos_t x, pos_t y, split_t type)
{
    LOG(DEBUG) << "SetView x=" << x << " y=" << y << " t=" << static_cast<int>(type);

    if (!x || !y)
        return true;

    if (y < 3)
        y = 3;
    if (y > m_sizey - m_topLines - m_bottomLines - 4)
        y = m_sizey - m_topLines - m_bottomLines - 4;

    if (x < 15)
        x = 15;
    if (x > m_sizex - 16)
        x = m_sizex - 16;

    if (type != split_t::split_v && type != split_t::split_h)
        type = split_t::no_split;

    m_splitX = x;
    m_splitY = y;
    m_splitType = type;

    CalcView();
    return 0;
}

bool WndManager::TrackView(const std::string& msg)
{
    if (m_splitType == split_t::no_split)
        return true;

    Application::getInstance().SetHelpLine(msg);

    if (m_splitType == split_t::split_h)
        InvColorRect(m_view[2].left, m_view[2].top - 1, m_view[2].sizex, 1);
    else if (m_splitType == split_t::split_v)
        InvColorRect(m_view[2].left - SPLIT_WIDTH, m_view[2].top, SPLIT_WIDTH, m_view[2].sizey);

    bool loop = true;
    while (loop)
    {
        input_t iKey = m_console.InputPending(500ms);

        Application::getInstance().PrintClock();
        if (iKey)
        {
            iKey = m_console.GetInput();
            LOG(DEBUG) << "TrackView " << std::hex << iKey << std::dec;
            if (iKey == K_ESC || iKey == K_ENTER || iKey == K_SPACE)
                break;

            if ((iKey & K_TYPEMASK) == K_RESIZE)
            {
                Application::getInstance().SetHelpLine();
                return Resize(K_GET_X(iKey), K_GET_Y(iKey));
            }

            pos_t x = m_splitX;
            pos_t y = m_splitY;

            if (iKey == K_LEFT || iKey == K_UP)
            {
                if (m_splitType == split_t::split_h)
                    --y;
                else
                    --x;
            }
            else if (iKey == K_RIGHT || iKey == K_DOWN)
            {
                if (m_splitType == split_t::split_h)
                    ++y;
                else
                    ++x;
            }

            if ((iKey & K_MOUSE) == K_MOUSE)
            {
                //mouse event
                x = K_GET_X(iKey);
                y = K_GET_Y(iKey) - m_topLines;

                if ((iKey & K_TYPEMASK) == K_MOUSEKUP)
                    loop = false;
            }

            if (x != m_splitX || y != m_splitY)
            {
                if (m_splitType == split_t::split_h)
                    InvColorRect(m_view[2].left, m_view[2].top - 1, m_view[2].sizex, 1);
                else if (m_splitType == split_t::split_v)
                    InvColorRect(m_view[2].left - SPLIT_WIDTH, m_view[2].top, SPLIT_WIDTH, m_view[2].sizey);

                if (m_splitType == split_t::split_h)
                    SetView(m_splitX, y, m_splitType);
                else
                    SetView(x, m_splitY, m_splitType);

                if (m_splitType == split_t::split_h)
                    InvColorRect(m_view[2].left, m_view[2].top - 1, m_view[2].sizex, 1);
                else if (m_splitType == split_t::split_v)
                    InvColorRect(m_view[2].left - SPLIT_WIDTH, m_view[2].top, SPLIT_WIDTH, m_view[2].sizey);
            }
        }
    }

    bool rc = Application::getInstance().SetHelpLine()
    && Refresh();

    return rc;
}

bool WndManager::CheckInput(const std::chrono::milliseconds& waitTime)
{
    while (1)
    {
        m_console.InputPending(waitTime);
        input_t key = m_console.GetInput();
        LOG_IF(key, INFO) << "  " << ConsoleInput::CastKeyCode(key);
        if (key == K_SPACE)
            break;
    }
    return true;
}

