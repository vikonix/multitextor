#include "WndManager.h"

bool CaptureInput::InputCapture()
{
    if (!m_fCaptured)
    {
        LOG(DEBUG) << __func__ << this;
//???        m_pPrevCapture = g_pApplication->Capture(this);
        m_fCaptured = true;
    }
    return true;
}


bool CaptureInput::InputRelease()
{
    if (m_fCaptured)
    {
        LOG(DEBUG) << __func__ << this;
//???        g_pApplication->Capture(m_pPrevCapture);
        m_pPrevCapture = NULL;
        m_fCaptured = false;
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
    LOG(DEBUG) << __func__;
    if (!m_visible)
        return true;

    bool rc = DrawBorder()
           && Clr();
    return rc;
}

bool FrameWnd::GotoXY(pos_t x, pos_t y)
{
    LOG(DEBUG) << __func__ << "x=" << x << " y=" << y;
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
    if (m_sizex < 0)
        return WndManager::getInstance().GetView(this).sizex + m_sizex + 1 - m_left;
    else
        return m_sizex;
}

pos_t FrameWnd::GetWSizeY() const
{
    if (m_sizey < 0)
        return WndManager::getInstance().GetView(this).sizey + m_sizey + 1 - m_top;
    else
        return m_sizey;
}

pos_t FrameWnd::GetCSizeX() const
{
    pos_t size;
    if (m_sizex < 0)
        size = WndManager::getInstance().GetView(this).sizex + m_sizex + 1 - m_left;
    else
        size = m_sizex;
    if (m_border & BORDER_LEFT)  --size;
    if (m_border & BORDER_RIGHT) --size;
    return size;
}

pos_t FrameWnd::GetCSizeY() const
{
    pos_t size;
    if (m_sizey < 0)
        size = WndManager::getInstance().GetView(this).sizey + m_sizey + 1 - m_top;
    else
        size = m_sizey;
    if (m_border & (BORDER_TOP | BORDER_TITLE)) --size;
    if (m_border & BORDER_BOTTOM) --size;
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

