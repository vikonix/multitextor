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
#include "WndManager/WndManager.h"
#include "WndManager/App.h"

namespace _WndManager
{

static const pos_t SPLIT_WIDTH { 1 };

bool WndManager::Init()
{
    bool rc = m_console.Init();
    if (!rc)
        return false;

    m_console.GetScreenSize(m_sizex, m_sizey);

    LOG(INFO) << "  M::Init" << " x=" << m_sizex << " y=" << m_sizey;
    CalcView();

    m_screenBuff.SetSize(m_sizex, m_sizey);

    return true;
}

bool WndManager::Deinit()
{
    LOG(INFO) << "  M::Deinit";

    if (m_view[2].wnd)
        m_view[2].wnd = nullptr;

    m_wndList.clear();

    bool rc = true;
    if (0 != m_screenBuff.GetSize())
    {
        rc = m_console.SetTextAttr(DEFAULT_COLOR)
          && m_console.ClrScr();

        m_screenBuff.SetSize();
    }

    m_console.Deinit();
    return rc;
}

bool WndManager::CalcView()
{
    //LOG(DEBUG) << __FUNC__ << " t=" << static_cast<int>(m_splitType);

    //for dialogs
    m_view[0].left  = 0;
    m_view[0].top   = m_topLines;
    m_view[0].sizex = m_sizex;
    m_view[0].sizey = m_sizey - m_topLines - m_bottomLines;

    //for work windows
    if (m_splitType == split_t::split_h)
    {
        //horizontal
        if (m_splitY > m_sizey - m_topLines - m_bottomLines - c_minSplitY - 1)
            m_splitY = m_sizey - m_topLines - m_bottomLines - c_minSplitY - 1;
        if (m_splitY <= 0)
        {
            ChangeViewMode();
            return true;
        }

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
        if (m_splitX > m_sizex - c_minSplitX)
            m_splitX = m_sizex - c_minSplitX;
        if (m_splitX <= 0)
        {
            ChangeViewMode();
            return true;
        }

        m_view[1].left  = 0;
        m_view[1].top   = m_topLines;
        m_view[1].sizex = m_splitX;
        m_view[1].sizey = m_sizey - m_topLines - m_bottomLines;
        if(m_wndList[0])
            m_wndList[0]->m_cursorx = 0;

        m_view[2].left  = m_splitX + SPLIT_WIDTH;
        m_view[2].top   = m_topLines;
        m_view[2].sizex = m_sizex - m_splitX - SPLIT_WIDTH;
        m_view[2].sizey = m_sizey - m_topLines - m_bottomLines;
        if(m_view[2].wnd)
            m_view[2].wnd->m_cursorx = 0;
    }

    return true;
}

bool WndManager::Refresh()
{
    //LOG(DEBUG) << "  M::Refresh";

    if (!m_sizex || !m_sizey)
        return false;

    m_disablePaint = 1;
    m_screenBuff.Fill(0);

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
            x = (m_sizex - static_cast<pos_t>(m_logo.logoStr.front().size())) / 2;
        if (y < 0)
            y = (m_sizey - static_cast<pos_t>(m_logo.logoStr.size())) / 2;

        SetTextAttr(m_logo.logoColor);

        if(x > 0 && y > 0)
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
    //LOG(DEBUG) << "  M::Cls";

    HideCursor();
    bool rc = CallConsole(ClrScr());
    return rc;
}


bool WndManager::SetTextAttr(color_t color)
{
    //LOG(DEBUG) << __FUNC__ << " c=" << std::hex << color << std::dec;
    _assert(TEXT_COLOR(color) != FON_COLOR(color));
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
    int rc{ true };
    if (m_cursor != cursor_t::CURSOR_HIDE)
    {
        rc = m_console.SetCursor(cursor_t::CURSOR_HIDE);
        m_cursor = cursor_t::CURSOR_HIDE;
    }
    return rc;
}

bool WndManager::ShowBuff()
{
    if (0 == m_screenBuff.GetSize())
        return false;

    bool rc = WriteBlock(0, 0, m_sizex - 1, m_sizey - 1, m_screenBuff);
    return rc;
}

bool WndManager::ShowBuff(pos_t left, pos_t top, pos_t sizex, pos_t sizey)
{
    if (0 == m_screenBuff.GetSize())
        return false;

    bool rc = false;
    if (left < 0 || top < 0 || left + sizex > m_sizex || top + sizey > m_sizey)
        LOG(ERROR) << __FUNC__ << "  M::ShowBuff l=" << left << " t=" << top << " sx=" << sizex << " sy=" << sizey;
    else
        rc = WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_screenBuff);
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
            block.push_back(m_screenBuff.GetCell(x, y));
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
            m_screenBuff.SetCell(x, y, block[i++]);

    bool rc = WriteBlock(left, top, right, bottom, m_screenBuff);
    return rc;
}

bool WndManager::WriteStr(const std::string& str)
{
    //LOG(DEBUG) << __FUNC__ << " '" << str << "' x=" << m_cursorx << " y=" << m_cursory << " c=" << m_color << " p=" << !m_disablePaint;
    std::u16string wstr = utf8::utf8to16(str);
    return WriteWStr(wstr);
}
    
bool WndManager::WriteWStr(const std::u16string& wstr)
{
    HideCursor();

    size_t l = wstr.size();
    bool rc;
    if (m_cursory != m_sizey - 1 || m_cursorx + static_cast<pos_t>(l) <= m_sizex - 1)
        rc = CallConsole(WriteStr(wstr));
    else
    {
        char16_t PrevC = wstr[l - 2];
        char16_t LastC = wstr[l - 1];
        rc = CallConsole(WriteStr(wstr.substr(0, l - 1)));
        rc = CallConsole(WriteLastChar(PrevC, LastC));
    }

    for(const auto& c : wstr)
        m_screenBuff.SetCell(m_cursorx++, m_cursory, MAKE_CELL(0, m_color, c));

    return rc;
}

bool WndManager::WriteColorWStr(const std::u16string& str, const std::vector<color_t>& color)
{
    HideCursor();

    pos_t x = m_cursorx;
    pos_t y = m_cursory;
    pos_t len = static_cast<pos_t>(str.size());
    for(pos_t i = 0; i < len; ++i)
        m_screenBuff.SetCell(m_cursorx++, m_cursory, MAKE_CELL(0, color[i], str[i]));

    bool rc = CallConsole(WriteBlock(x, y, x + len - 1, y, m_screenBuff, x, y));
    return rc;
}

bool WndManager::WriteColor(pos_t x, pos_t y, const std::vector<color_t>& color)
{
    HideCursor();
    pos_t len = static_cast<pos_t>(color.size());

    for (pos_t i = 0; i < len; ++i)
    {
        m_screenBuff.SetColor(x + i, y, color[i]);
    }

    bool rc = CallConsole(WriteBlock(x, y, x + len - 1, y, m_screenBuff, x, y));
    return rc;
}

bool WndManager::Resize(pos_t sizex, pos_t sizey)
{
    LOG(INFO) << "  M::Resize x=" << sizex << " y=" << sizey;
    _assert(sizex > 0 && sizey > 0);
    if (m_sizex == sizex && m_sizey == sizey)
        return true;

    if (sizex < c_minSplitX || sizey < c_minSplitY)
    {
        LOG(ERROR) << "Too small screen size";
        throw std::runtime_error("Too small screen size");
    }

    m_sizex = sizex;
    m_sizey = sizey;
    CalcView();

    m_screenBuff.SetSize(m_sizex, m_sizey);

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
    return true;
}

bool WndManager::WriteConsoleTitle(bool set)
{
    if (!set && !m_invalidTitle)
        return true;

    std::string name;
    if (!m_wndList.empty())
    {
        if (m_view[2].wnd && m_activeView == 1)
            name = m_view[2].wnd->GetObjectName();
        else
        {
            if (m_wndList[0]->GetWndType() == wnd_t::editor)
                name = m_wndList[0]->GetObjectName();
            else
                return true;
        }
    }

    bool rc = m_console.WriteConsoleTitle(name + " - " + Application::getInstance().m_appName);

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
    //LOG(DEBUG) << __FUNC__ << std::hex << c << std::dec;
    HideCursor();
    bool rc = true;
    if (m_cursory != m_sizey - 1 || m_cursorx < m_sizex - 1)
        rc = CallConsole(WriteChar(c));
    else
    {
        char16_t PrevC = GET_CTEXT(m_screenBuff.GetCell(static_cast<size_t>(m_cursorx) - 1, m_cursory));
        rc = CallConsole(WriteLastChar(PrevC, c));
    }

    m_screenBuff.SetCell(m_cursorx++, m_cursory, MAKE_CELL(0, m_color, c));

    return rc;
}

bool WndManager::FillRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, input_t c, color_t color)
{
    //LOG(DEBUG) << "  M::FillRect l=" << left << " t=" << top << " sx=" << sizex << " sy=" << sizey 
    //    << " ch=" << std::hex << c << " color=" << static_cast<int>(color) << std::dec;

    HideCursor();
    cell_t cl = MAKE_CELL(0, color, c);
    for (pos_t y = 0; y < sizey; ++y)
    {
        for (pos_t x = 0; x < sizex; ++x)
        {
            m_screenBuff.SetCell(static_cast<size_t>(left) + x, static_cast<size_t>(top) + y, cl);
        }
    }

    bool rc = CallConsole(WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_screenBuff, left, top));
    return rc;
}

bool WndManager::ColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, color_t color)
{
    //LOG(DEBUG) << "  M::ColorRect l=" << left << " t=" << top << " x=" << sizex << " y=" << sizey << " color=" << color;

    HideCursor();
    for (pos_t y = 0; y < sizey; ++y)
    {
        for (pos_t x = 0; x < sizex; ++x)
        {
            m_screenBuff.SetColor(static_cast<size_t>(left) + x, static_cast<size_t>(top) + y, color);
        }
    }

    int rc = CallConsole(WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_screenBuff, left, top));
    return rc;
}

bool WndManager::InvColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey)
{
    //LOG(DEBUG) << "  M::InvColorRect l=" << left << " t=" << top << " sx=" << sizex << " sy=" << sizey;

    HideCursor();
    for (pos_t y = 0; y < sizey; ++y)
    {
        for (pos_t x = 0; x < sizex; ++x)
        {
            color_t color = COLOR_INVERSE(GET_CCOLOR(m_screenBuff.GetCell(static_cast<size_t>(left) + x, static_cast<size_t>(top) + y)));
            m_screenBuff.SetColor(static_cast<size_t>(left) + x, static_cast<size_t>(top) + y, color);
        }
    }

    int rc = CallConsole(WriteBlock(left, top, left + sizex - 1, top + sizey - 1, m_screenBuff, left, top));
    return rc;
}

bool WndManager::Show(Wnd* wnd, bool refresh, int view)
{
    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd << " refresh=" << refresh << " view=" << view;

    //save current state before change wnd list
    bool inv = m_invalidate;
    if (0 == view)
    {
        m_activeView = 0;
        AddWnd(wnd);
    }
    else
        SetTopWnd(wnd, view);

    if (!inv && !refresh)
        m_invalidate = false;

    return true;
}

bool WndManager::Hide(Wnd* wnd, bool refresh)
{
    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd << " refresh=" << refresh;

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

    //LOG(DEBUG) << __FUNC__ << " n=" << view;

    return m_view[view];
}

bool WndManager::CloneView(Wnd* wnd)
{
    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd;

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
        m_activeView = 0;
        m_splitType = split_t::no_split;
        m_view[2].wnd = nullptr;
    }
    else
    {
        m_view[2].wnd = wnd->CloneWnd();
    }

    return true;
}

bool WndManager::AddWnd(Wnd* wnd)
{
    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd;

    //add to top
    m_wndList.push_front(wnd);
    m_invalidate = true;
    m_invalidTitle = true;
    return true;
}

bool WndManager::AddLastWnd(Wnd* wnd)
{
    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd;
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
    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd;
    if (nullptr == wnd)
        return true;

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
    if (m_wndList.empty() && nullptr != m_view[2].wnd)
    {
        if (!m_activeView)
        {
            SetTopWnd(m_view[2].wnd);
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


Wnd* WndManager::GetWnd(size_t n, int view)
{
    if (view == -1)
        view = m_activeView;

    if (view == 1)
        return m_view[2].wnd;

    if (n < m_wndList.size())
        return m_wndList[n];
    else
        return nullptr;
}

bool WndManager::SetTopWnd(int n, int view)
{
    //LOG(DEBUG) << __FUNC__ << " n=" << n << " view=" <<view;

    if ((n == 0 && view == 0) || (n < 0 && view != 0))
        return true;

    if (n < 0)
        return SetTopWnd(m_view[2].wnd);
    else if (n < static_cast<int>(m_wndList.size()))
        return SetTopWnd(m_wndList[n], view);
    else
        return false;
}

bool WndManager::SetTopWnd(Wnd* wnd, int view)
{
    if (!wnd)
        return true;

    //LOG(DEBUG) << __FUNC__ << " wnd=" << wnd << " view=" << view;
    if (!m_wndList.empty())
    {
        //LOG(DEBUG) << "top=" << m_wndList[0] << " view2=" << m_view[2].wnd << " aview=" << m_activeView;
    }

    if (split_t::no_split == m_splitType)
        view = 0;
    else if (view == -1)
        view = m_activeView;

    if (!view)
    {
        if (!m_wndList.empty() && wnd == m_wndList[0])
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

bool WndManager::IsVisible(const Wnd* wnd)
{
    return wnd == m_view[2].wnd 
        || (!m_wndList.empty() && wnd == m_wndList[0])
        || (m_wndList.size() > 1 && m_wndList[0]->GetWndType() == wnd_t::dialog && m_wndList[1] == wnd);
}

input_t WndManager::ProcInput(input_t code)
{
//    LOG_IF(code != K_TIME, DEBUG) << "  M:ProcInput " <<  std::hex << code << std::dec;

    input_t out{code};
    [[maybe_unused]]bool rc{};

    if (code == K_REFRESH)
    {
        //refresh
        //LOG(DEBUG) << "WndManager Refresh";
        Refresh();
        out = 0;
    }
    else if ((code & K_TYPEMASK) == K_RESIZE)
    {
        rc = Resize(K_GET_X(code), K_GET_Y(code));
        out = 0;
    }
    else
    {
        if (!m_wndList.empty())
        {
            if ((code & K_MOUSE) == K_MOUSE)
            {
                //mouse event
                if ((code & K_TYPEMASK) == K_MOUSEKUP)
                {
                    if (!m_activeView)
                        out = m_wndList[0]->EventProc(code);
                    else
                        out = m_view[2].wnd->EventProc(code);
                }
                else if ((code & K_MOUSEW) == K_MOUSEW)
                {
                    pos_t x = K_GET_X(code);
                    pos_t y = K_GET_Y(code);
                    if (m_view[2].wnd == nullptr || m_wndList[0]->CheckWndPos(x, y))
                    {
                        out = m_wndList[0]->EventProc(code);
                    }
                    else if (m_view[2].wnd)
                    {
                        out = m_view[2].wnd->EventProc(code);
                    }
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
                            out = m_wndList[0]->EventProc(code);
                    }
                    else if (m_view[2].wnd)
                    {
                        if (m_view[2].wnd->CheckWndPos(x, y))
                        {
                            SetActiveView(1);
                            m_view[2].wnd->ScreenToClient(x, y);
                            if (m_view[2].wnd->CheckClientPos(x, y))
                                out = m_view[2].wnd->EventProc(code);
                        }
                        else
                        {
                            //LOG(DEBUG) << "Split line";
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
                        out = m_wndList[0]->EventProc(code);
                    else
                        out = m_view[2].wnd->EventProc(code);
                }
                else if (m_wndList[0]->IsUsedTimer())
                {
                    if (m_activeView == 1 && m_view[2].wnd && m_view[2].wnd->IsUsedTimer())
                        m_view[2].wnd->EventProc(code);

                    for(auto& wnd : m_wndList)
                        wnd->EventProc(code);

                    if (m_activeView != 1 && m_view[2].wnd && m_view[2].wnd->IsUsedTimer())
                        m_view[2].wnd->EventProc(code);
                }
            }
        }
    }

    return out;
}

bool WndManager::SetActiveView(int n)
{
    //LOG(DEBUG) << "SetActiveView st=" << static_cast<int>(m_splitType) << " av=" << m_activeView << " n=" << n;

    if (m_splitType == split_t::no_split)
        return true;

    if (n < 0)
        m_activeView = m_activeView ? 0 : 1;
    else if (n <= 1)
        m_activeView = n;
    else
        m_activeView = 0;

    return WriteConsoleTitle(true);
}

bool WndManager::ChangeViewMode(int mode)//0-create/del 1-horiz/vert
{
    //LOG(DEBUG) << "ChangeViewMode split type=" << static_cast<int>(m_splitType) << " mode=" << mode;

    if (m_wndList.empty())
        return true;

    if (!m_splitX || !m_splitY)
    {
        m_splitX = m_sizex / 2;
        m_splitY = m_sizey / 2;
    }

    if (mode == 0)
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
            return true;

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
    //LOG(DEBUG) << "SetView x=" << x << " y=" << y << " type=" << static_cast<int>(type);

    if (!x || !y)
        return true;

    if (y < c_minSplitY)
        y = c_minSplitY;
    if (y > m_sizey - m_topLines - m_bottomLines - c_minSplitY - 1)
        y = m_sizey - m_topLines - m_bottomLines - c_minSplitY - 1;

    if (x < c_minSplitX - 1)
        x = c_minSplitX - 1;
    if (x > m_sizex - c_minSplitX)
        x = m_sizex - c_minSplitX;

    if (type != split_t::split_v && type != split_t::split_h)
        type = split_t::no_split;

    m_splitX = x;
    m_splitY = y;
    m_splitType = type;

    CalcView();
    return true;
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
            //LOG(DEBUG) << "TrackView " << std::hex << iKey << std::dec;
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

input_t WndManager::CheckInput(const std::chrono::milliseconds& waitTime)
{
    bool key = m_console.InputPending(waitTime);

    Application::getInstance().PrintClock();
    if (key)
    {
        input_t code = m_console.GetInput();
        if (code == K_FOCUSLOST || code == K_FOCUSSET)
            code = 0;

        LOG_IF(code, DEBUG) << "  " << ConsoleInput::CastKeyCode(code);
        return code;
    }

    return 0;
}

bool WndManager::Scroll(pos_t left, pos_t top, pos_t right, pos_t bottom, pos_t n, scroll_t mode)
{
    uint32_t invalidate{};

    HideCursor();
    bool rc = CallConsole(ScrollBlock(left, top, right, bottom, n, mode, &invalidate));
    rc = m_screenBuff.ScrollBlock(left, top, right, bottom, n, mode);

    if ((invalidate & INVALIDATE_LEFT) && left > 0)
        rc = WriteBlock(0, top, left - 1, bottom, m_screenBuff);

    if ((invalidate & INVALIDATE_RIGHT) && right < m_sizex - 1)
        rc = WriteBlock(right + 1, top, m_sizex - 1, bottom, m_screenBuff);

    return rc;
}

} //namespace _WndManager 
