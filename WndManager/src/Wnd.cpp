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
#include "WndManager.h"
#include "App.h"


bool CaptureInput::InputCapture()
{
    if (!m_captured)
    {
        LOG(DEBUG) << __FUNC__ << " " << this;
        m_prevCaptured = Application::getInstance().Capture(this);
        m_captured = true;
    }
    return true;
}

bool CaptureInput::InputRelease()
{
    if (m_captured)
    {
        LOG(DEBUG) << __FUNC__ << " " << this;
        Application::getInstance().Capture(m_prevCaptured);
        m_prevCaptured = nullptr;
        m_captured = false;
    }
    return true;
}

bool Wnd::Show(bool refresh, int view)
{
    bool rc = 0;
    if (!m_visible)
    {
        m_visible = true;
        rc = WndManager::getInstance().Show(this, refresh, view);
    }
    return rc;
}

bool Wnd::Hide(bool refresh)
{
    bool rc = 0;
    if (m_visible)
    {
        m_visible = false;
        rc = WndManager::getInstance().Hide(this, refresh);
    }
    return rc;
}

void  Wnd::StopPaint()
{
    WndManager::getInstance().StopPaint();
}

void  Wnd::BeginPaint()
{
    WndManager::getInstance().BeginPaint();
}

void Wnd::ClientToScreen(pos_t& x, pos_t& y) const
{
    x += m_left;
    y += m_top;
}

void Wnd::ScreenToClient(pos_t& x, pos_t& y) const
{
    x -= m_left;
    y -= m_top;
}

bool Wnd::CheckClientPos(pos_t x, pos_t y) const
{
    if (x >= 0 && x < m_sizex && y >= 0 && y < m_sizey)
        return true;
    else
        return false;
}

//////////////////////////////////////////////////////////////////////////////
bool FrameWnd::Refresh()
{
    LOG(DEBUG) << __FUNC__;
    if (!m_visible)
        return true;

    bool rc = DrawBorder()
        && Clr();
    return rc;
}

bool FrameWnd::GotoXY(pos_t x, pos_t y)
{
    //LOG(DEBUG) << __FUNC__ << " x=" << x << " y=" << y;
    m_cursorx = x;
    m_cursory = y;

    return true;
}

void FrameWnd::GetWindowSize(pos_t& sizeX, pos_t& sizeY) const
{
    sizeX = GetWSizeX();
    sizeY = GetWSizeY();
}

void FrameWnd::ClientToScreen(pos_t& x, pos_t& y) const
{
    x += WndManager::getInstance().GetView(this).left + m_left + ((m_border & BORDER_LEFT) ? 1 : 0);
    y += WndManager::getInstance().GetView(this).top + m_top + ((m_border & (BORDER_TITLE | BORDER_TOP)) ? 1 : 0);
}

void FrameWnd::ScreenToClient(pos_t& x, pos_t& y) const
{
    x -= WndManager::getInstance().GetView(this).left + m_left + ((m_border & BORDER_LEFT) ? 1 : 0);
    y -= WndManager::getInstance().GetView(this).top + m_top + ((m_border & (BORDER_TITLE | BORDER_TOP)) ? 1 : 0);
}

bool FrameWnd::CheckClientPos(pos_t x, pos_t y) const
{
    if (x >= 0 && x < GetCSizeX() && y >= 0 && y < GetCSizeY())
        return true;
    else
        return false;
}

bool FrameWnd::CheckWndPos(pos_t x, pos_t y) const
{
    x -= WndManager::getInstance().GetView(this).left + m_left;
    y -= WndManager::getInstance().GetView(this).top + m_top;

    if (x >= 0 && x < GetWSizeX() && y >= 0 && y < GetWSizeY())
        return true;
    else
        return false;
}

pos_t FrameWnd::GetWSizeX() const
{
    if (m_sizex <= 0)
        return WndManager::getInstance().GetView(this).sizex - m_left;
    else
        return m_sizex;
}

pos_t FrameWnd::GetWSizeY() const
{
    if (m_sizey <= 0)
        return WndManager::getInstance().GetView(this).sizey - m_top;
    else
        return m_sizey;
}

pos_t FrameWnd::GetCSizeX() const
{
    pos_t size;
    if (m_sizex <= 0)
        size = WndManager::getInstance().GetView(this).sizex - m_left;
    else
        size = m_sizex;

    if (m_border & BORDER_LEFT)  
        --size;
    if (m_border & BORDER_RIGHT) 
        --size;
    return size;
}

pos_t FrameWnd::GetCSizeY() const
{
    pos_t size;
    if (m_sizey <= 0)
        size = WndManager::getInstance().GetView(this).sizey - m_top;
    else
        size = m_sizey;

    if (m_border & (BORDER_TOP | BORDER_TITLE)) 
        --size;
    if (m_border & BORDER_BOTTOM) 
        --size;
    return size;
}

bool FrameWnd::Clr()
{
    if (!m_visible)
        return true;

    pos_t cleft = 0;
    pos_t ctop = 0;
    ClientToScreen(cleft, ctop);

    bool rc = WndManager::getInstance().FillRect(cleft, ctop, GetCSizeX(), GetCSizeY(), ' ', *m_pColorWindow);
    return rc;
}

bool FrameWnd::DrawBorder()
{
    if (!m_visible)
        return true;

    if (m_border == NO_BORDER)
        return true;

    WndManager& manager = WndManager::getInstance();
    pos_t wleft = m_left + manager.GetView(this).left;
    pos_t wtop = m_top + manager.GetView(this).top;
    pos_t wsizex = GetWSizeX();
    pos_t wsizey = GetWSizeY();

    bool rc = true;
    if (m_border & BORDER_LINE)
    {
        //border with line
        rc = manager.SetTextAttr(*m_pColorWindowBorder);
        if (m_border & BORDER_TOP)
        {
            rc = manager.GotoXY(wleft, wtop);
            rc = manager.WriteWChar(ACS_ULCORNER);
            rc = manager.FillRect(wleft + 1, wtop, wsizex - 2, 1, ACS_HLINE, *m_pColorWindowBorder);
            rc = manager.GotoXY(wleft + wsizex - 1, wtop);
            rc = manager.WriteWChar(ACS_URCORNER);
        }
        if (m_border & BORDER_LEFT)
            rc = manager.FillRect(wleft, wtop + 1, 1, wsizey - 2, ACS_VLINE, *m_pColorWindowBorder);
        if (m_border & BORDER_RIGHT)
            rc = manager.FillRect(wleft + wsizex - 1, wtop + 1, 1, wsizey - 2, ACS_VLINE, *m_pColorWindowBorder);
        if (m_border & BORDER_BOTTOM)
        {
            rc = manager.GotoXY(wleft, wtop + wsizey - 1);
            rc = manager.WriteWChar(ACS_LLCORNER);
            rc = manager.FillRect(wleft + 1, wtop + wsizey - 1, wsizex - 2, 1, ACS_HLINE, *m_pColorWindowBorder);
            rc = manager.GotoXY(wleft + wsizex - 1, wtop + wsizey - 1);
            rc = manager.WriteWChar(ACS_LRCORNER);
        }

        if (m_border & BORDER_TITLE)
        {
            if (m_border & BORDER_TOP)
                rc = manager.FillRect(wleft + 2, wtop, wsizex - 4, 1, ' ', *m_pColorWindowTitle);
            else
                rc = manager.FillRect(wleft, wtop, wsizex, 1, ' ', *m_pColorWindowTitle);
        }
    }
    else
    {
        //border without line
        if (m_border & BORDER_TOP)
            rc = manager.FillRect(wleft, wtop, wsizex, 1, ' ', *m_pColorWindowBorder);
        if (m_border & BORDER_LEFT)
            rc = manager.FillRect(wleft, wtop, 1, wsizey, ' ', *m_pColorWindowBorder);
        if (m_border & BORDER_RIGHT)
            rc = manager.FillRect(wleft + wsizex - 1, wtop, 1, wsizey, ' ', *m_pColorWindowBorder);
        if (m_border & BORDER_BOTTOM)
            rc = manager.FillRect(wleft, wtop + wsizey - 1, wsizex, 1, ' ', *m_pColorWindowBorder);

        if (m_border & BORDER_TITLE)
        {
            if (m_border & BORDER_TOP)
                rc = manager.FillRect(wleft + 2, wtop, wsizex - 4, 1, ' ', *m_pColorWindowTitle);
            else
                rc = manager.FillRect(wleft, wtop, wsizex, 1, ' ', *m_pColorWindowTitle);
        }
    }

    return rc;
}

bool FrameWnd::WriteWnd(pos_t x, pos_t y, const std::string& str, color_t color)
{
    if (!m_visible)
        return true;

    if (x == MAX_COORD || y == MAX_COORD || x > GetWSizeX() || y > GetWSizeY())
        return true;
    
    pos_t wleft = m_left + WndManager::getInstance().GetView(this).left + x;
    pos_t wtop = m_top + WndManager::getInstance().GetView(this).top + y;
    //LOG(DEBUG) << "WriteWnd wx=" << x << " wy=" << y << " x=" << wleft << " y=" << wtop << " s=" << str;

    bool rc = WndManager::getInstance().GotoXY(wleft, wtop);
    rc = WndManager::getInstance().SetTextAttr(color);
    if((pos_t)str.size() <= GetWSizeX() - (wleft - m_left))
        rc = WndManager::getInstance().WriteStr(str);
    else
        rc = WndManager::getInstance().WriteStr(str.substr(0, (size_t)GetWSizeX() - (wleft - m_left)));

    return rc;
}

bool FrameWnd::WriteStr(pos_t x, pos_t y, const std::string& str, color_t color)
{
    if (!m_visible)
        return true;

    if (x == MAX_COORD || y == MAX_COORD || x > GetCSizeX() || y > GetCSizeY())
        return true;

    ClientToScreen(x, y);

    bool rc = WndManager::getInstance().GotoXY(x, y);
    rc = WndManager::getInstance().SetTextAttr(color);
    rc = WndManager::getInstance().WriteStr(str);

    return rc;
}

bool FrameWnd::WriteWStr(pos_t x, pos_t y, const std::u16string& str, color_t color)
{
    if (!m_visible)
        return true;

    if (x == MAX_COORD || y == MAX_COORD || x > GetCSizeX() || y > GetCSizeY())
        return true;

    ClientToScreen(x, y);

    bool rc = WndManager::getInstance().GotoXY(x, y);
    rc = WndManager::getInstance().SetTextAttr(color ? color : m_color);
    rc = WndManager::getInstance().WriteWStr(str);

    return rc;
}

bool FrameWnd::WriteChar(pos_t x, pos_t y, char c, color_t color)
{
    if (!m_visible)
        return true;

    if (x == MAX_COORD || y == MAX_COORD || x > GetCSizeX() || y > GetCSizeY())
        return true;

    ClientToScreen(x, y);

    bool rc = WndManager::getInstance().GotoXY(x, y);
    rc = WndManager::getInstance().SetTextAttr(color);
    rc = WndManager::getInstance().WriteChar(c);

    return rc;
}

bool FrameWnd::WriteWChar(pos_t x, pos_t y, char16_t c, color_t color)
{
    if (!m_visible)
        return true;

    if (x == MAX_COORD || y == MAX_COORD || x > GetCSizeX() || y > GetCSizeY())
        return true;

    ClientToScreen(x, y);

    bool rc = WndManager::getInstance().GotoXY(x, y);
    rc = WndManager::getInstance().SetTextAttr(color);
    rc = WndManager::getInstance().WriteWChar(c);

    return rc;
}

bool FrameWnd::FillRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, input_t c, color_t color)
{
    if (!m_visible)
        return true;

    ClientToScreen(left, top);

    bool rc = WndManager::getInstance().FillRect(left, top, sizex, sizey, c, color);
    return rc;
}

bool FrameWnd::ShowBuff(pos_t left, pos_t top, pos_t sizex, pos_t sizey)
{
    if (!m_visible)
        return true;

    ClientToScreen(left, top);
    return WndManager::getInstance().ShowBuff(left, top, sizex, sizey);
}

bool FrameWnd::ColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, color_t color)
{
    if (!m_visible)
        return true;

    ClientToScreen(left, top);

    bool rc = WndManager::getInstance().ColorRect(left, top, sizex, sizey, color);
    return rc;
}

bool FrameWnd::WriteColor(pos_t x, pos_t y, const std::vector<color_t>& color)
{
    if (!m_visible)
        return 0;

    ClientToScreen(x, y);

    bool rc = WndManager::getInstance().WriteColor(x, y, color);
    return rc;
}

bool FrameWnd::WriteColorStr(pos_t x, pos_t y, const std::u16string& str, const std::vector<color_t>& color)
{
    if (!m_visible)
        return true;

    if (x < 0 || y < 0 || x > GetCSizeX() || y > GetCSizeY())
        return 0;

    ClientToScreen(x, y);

    bool rc = WndManager::getInstance().GotoXY(x, y);
    rc = WndManager::getInstance().WriteColorWStr(str, color);
    return rc;
}

bool FrameWnd::PutMacro(input_t cmd) 
{ 
    return Application::getInstance().PutMacro(cmd);
}

bool FrameWnd::Scroll(pos_t n, scroll_t mode)
{
    if (!m_visible)
        return true;

    pos_t left = 0;
    pos_t top = 0;
    pos_t right = GetCSizeX() - 1;
    pos_t bottom = GetCSizeY() - 1;

    ClientToScreen(left, top);
    ClientToScreen(right, bottom);

    bool rc = WndManager::getInstance().SetTextAttr(*m_pColorWindow)
           && WndManager::getInstance().Scroll(left, top, right, bottom, n, mode);
    return rc;
}

bool FrameWnd::Beep()
{
    return WndManager::getInstance().Beep();
}
