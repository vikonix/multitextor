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
#include "WndShade.h"
#include "WndManager.h"
#include "utils/logger.h"


Shade::Shade(pos_t x, pos_t y, pos_t sizex, pos_t sizey, int mode)
{
    //LOG(DEBUG) << __FUNC__;

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
        if(y < 1)
            m_mode &= ~SHADE_TOP;

    if(m_mode & SHADE_LEFT)
        if(!x)
            m_mode &= ~SHADE_LEFT;

    if(m_mode & SHADE_RIGHT)
        if(x + sizex >= WndManager::getInstance().m_sizex)
            m_mode &= ~SHADE_RIGHT;

    if(m_mode & SHADE_BOTTOM)
        if(y + sizey > WndManager::getInstance().m_sizey - 2)
            m_mode &= ~SHADE_BOTTOM;

    if (m_mode & SHADE_LEFT)
    {
        --m_x;
        ++m_sizex;
    }
    if (m_mode & SHADE_RIGHT)
        ++m_sizex;
    
    if(m_mode & SHADE_SAVE)
    {
        if (m_mode & SHADE_TOP)
            WndManager::getInstance().GetBlock(m_x, m_y - 1, m_x + m_sizex - 1, m_y - 1, m_pSaveT);
        if(m_mode & SHADE_LEFT)
            WndManager::getInstance().GetBlock(m_x, m_y, m_x, m_y + m_sizey - 1, m_pSaveL);
        if(m_mode & SHADE_RIGHT)
            WndManager::getInstance().GetBlock(m_x + m_sizex - 1, m_y, m_x + m_sizex - 1, m_y + m_sizey - 1, m_pSaveR);
        if (m_mode & SHADE_BOTTOM)
            WndManager::getInstance().GetBlock(m_x, m_y + m_sizey, m_x + m_sizex - 1, m_y + m_sizey, m_pSaveB);
    }

    if(m_mode & SHADE_PAINT)
        Paint();
}

Shade::~Shade()
{
    //LOG(DEBUG) << __FUNC__;

    if(m_mode & SHADE_SAVE)
        Hide();
}

bool Shade::Hide()
{
    //LOG(DEBUG) << __FUNC__;

    WndManager::getInstance().PutBlock(m_x, m_y - 1, m_x + m_sizex - 1, m_y - 1, m_pSaveT);
    WndManager::getInstance().PutBlock(m_x, m_y, m_x, m_y + m_sizey - 1, m_pSaveL);
    WndManager::getInstance().PutBlock(m_x + m_sizex - 1, m_y, m_x + m_sizex - 1, m_y + m_sizey - 1, m_pSaveR);
    WndManager::getInstance().PutBlock(m_x, m_y + m_sizey, m_x + m_sizex - 1, m_y + m_sizey, m_pSaveB);

    return true;
}

bool Shade::Paint()
{
    //LOG(DEBUG) << __FUNC__;

    color_t color = ColorShade;

    if (m_mode & SHADE_TOP)
        WndManager::getInstance().ColorRect(m_x, m_y - 1, m_sizex, 1, color);
    if(m_mode & SHADE_LEFT)
        WndManager::getInstance().ColorRect(m_x, m_y, 1, m_sizey, color);
    if(m_mode & SHADE_RIGHT)
        WndManager::getInstance().ColorRect(m_x + m_sizex - 1, m_y, 1, m_sizey, color);
    if (m_mode & SHADE_BOTTOM)
        WndManager::getInstance().ColorRect(m_x, m_y + m_sizey, m_sizex, 1, color);

    return true;
}

