#include "WndManager.h"


bool Wnd::Show(bool refresh, int view)
{
    bool rc = 0;
    if (!m_fVisible)
    {
        m_fVisible = 1;
        rc = WndManager::getInstance().Show(this, refresh, view);
    }
    return rc;
}

bool Wnd::Hide(bool refresh)
{
    bool rc = 0;
    if (m_fVisible)
    {
        m_fVisible = 0;
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

