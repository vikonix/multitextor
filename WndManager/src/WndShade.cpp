#include "WndShade.h"
#include "WndManager.h"
#include "utils/logger.h"


Shade::Shade(pos_t x, pos_t y, pos_t sizex, pos_t sizey, int mode)
{
    LOG(DEBUG) << __FUNCTION__;

    m_x     = x;
    m_y     = y;
    m_sizex = sizex;
    m_sizey = sizey;
    m_mode  = mode;

    if(x < 0 || y < 0)
    {
        m_mode = 0;
        return;
    }

    if(m_mode & SHADE_TOP)
        if(y <= 1)
            m_mode &= ~SHADE_TOP;

    if(m_mode & SHADE_LEFT)
        if(!x)
            m_mode &= ~SHADE_LEFT;

    if(m_mode & SHADE_RIGHT)
        if(x + sizex > WndManager::getInstance().m_sizex - 1)
            m_mode &= ~SHADE_RIGHT;

    if(m_mode & SHADE_BOTTOM)
        if(y + sizey > WndManager::getInstance().m_sizey - 2)
            m_mode &= ~SHADE_BOTTOM;

    if(m_mode & SHADE_SAVE)
    {
        if(m_mode & SHADE_TOP)
          WndManager::getInstance().GetBlock(m_x, m_y - 1, m_x + m_sizex - 1, m_y - 1, m_pSaveT);
        if(m_mode & SHADE_LEFT)
          WndManager::getInstance().GetBlock(m_x - 1, m_y, m_x - 1, m_y + m_sizey - 1, m_pSaveL);
        if(m_mode & SHADE_RIGHT)
          WndManager::getInstance().GetBlock(m_x + m_sizex, m_y, m_x + m_sizex, m_y + m_sizey - 1, m_pSaveR);
        if(m_mode & SHADE_BOTTOM)
          WndManager::getInstance().GetBlock(m_x, m_y + m_sizey, m_x + m_sizex - 1, m_y + m_sizey, m_pSaveB);

        if((m_mode & SHADE_TOP) && (m_mode & SHADE_LEFT))
          WndManager::getInstance().GetBlock(m_x - 1, m_y - 1, m_x - 1, m_y - 1, m_SaveTL);
        if((m_mode & SHADE_TOP) && (m_mode & SHADE_RIGHT))
          WndManager::getInstance().GetBlock(m_x + m_sizex, m_y - 1, m_x + m_sizex, m_y - 1, m_SaveTR);
        if((m_mode & SHADE_BOTTOM) && (m_mode & SHADE_LEFT))
          WndManager::getInstance().GetBlock(m_x - 1, m_y + m_sizey, m_x - 1, m_y + m_sizey, m_SaveBL);
        if((m_mode & SHADE_BOTTOM) && (m_mode & SHADE_RIGHT))
          WndManager::getInstance().GetBlock(m_x + m_sizex, m_y + m_sizey, m_x + m_sizex, m_y + m_sizey, m_SaveBR);
    }

    if(m_mode & SHADE_PAINT)
        Paint();
}


Shade::~Shade()
{
    LOG(DEBUG) << __FUNCTION__;

    if(m_mode & SHADE_SAVE)
        Hide();
}


bool Shade::Paint()
{
    LOG(DEBUG) << __FUNCTION__;

    color_t color = ColorShade;

    if(m_mode & SHADE_TOP)
        WndManager::getInstance().ColorRect(m_x, m_y - 1, m_sizex, 1, color);
    if(m_mode & SHADE_LEFT)
        WndManager::getInstance().ColorRect(m_x - 1, m_y, 1, m_sizey, color);
    if(m_mode & SHADE_RIGHT)
        WndManager::getInstance().ColorRect(m_x + m_sizex, m_y, 1, m_sizey, color);
    if(m_mode & SHADE_BOTTOM)
        WndManager::getInstance().ColorRect(m_x, m_y + m_sizey, m_sizex, 1, color);

    if((m_mode & SHADE_TOP) && (m_mode & SHADE_LEFT))
        WndManager::getInstance().ColorRect(m_x - 1, m_y - 1, 1, 1, color);
    if((m_mode & SHADE_TOP) && (m_mode & SHADE_RIGHT))
        WndManager::getInstance().ColorRect(m_x + m_sizex, m_y - 1, 1, 1, color);
    if((m_mode & SHADE_BOTTOM) && (m_mode & SHADE_LEFT))
        WndManager::getInstance().ColorRect(m_x - 1, m_y + m_sizey, 1, 1, color);
    if((m_mode & SHADE_BOTTOM) && (m_mode & SHADE_RIGHT))
        WndManager::getInstance().ColorRect(m_x + m_sizex, m_y + m_sizey, 1, 1, color);

    return true;
}


bool Shade::Hide()
{
    LOG(DEBUG) << __FUNCTION__;

    WndManager::getInstance().PutBlock(m_x, m_y - 1, m_x + m_sizex - 1, m_y - 1, m_pSaveT);
    WndManager::getInstance().PutBlock(m_x - 1, m_y, m_x - 1, m_y + m_sizey - 1, m_pSaveL);
    WndManager::getInstance().PutBlock(m_x + m_sizex, m_y, m_x + m_sizex, m_y + m_sizey - 1, m_pSaveR);
    WndManager::getInstance().PutBlock(m_x, m_y + m_sizey, m_x + m_sizex - 1, m_y + m_sizey, m_pSaveB);

    WndManager::getInstance().PutBlock(m_x - 1, m_y - 1, m_x - 1, m_y - 1, m_SaveTL);
    WndManager::getInstance().PutBlock(m_x + m_sizex, m_y - 1, m_x + m_sizex, m_y - 1, m_SaveTR);
    WndManager::getInstance().PutBlock(m_x - 1, m_y + m_sizey, m_x - 1, m_y + m_sizey, m_SaveBL);
    WndManager::getInstance().PutBlock(m_x + m_sizex, m_y + m_sizey, m_x + m_sizex, m_y + m_sizey, m_SaveBR);

    return true;
}

