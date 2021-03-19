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
#include "Menu.h"
#include "WndManager.h"
#include "App.h"
#include "utils/logger.h"

#include <cwctype>

/////////////////////////////////////////////////////////////////////////////
input_t Menu::Close(input_t code)
{
    if(m_menu.empty())
        return 0;

    LOG(DEBUG) << __FUNC__ << " k=" << std::hex << code << std::dec;

    if(m_nextMenu)
    {
        m_nextMenu.reset();
    }

    if(m_shade)
    {
        if((code & K_TYPEMASK) == K_RESIZE)
            m_shade->Discard();
        m_shade.reset();
    }

    if(!m_saveBlock.empty())
    {
        if((code & K_TYPEMASK) != K_RESIZE)
            WndManager::getInstance().PutBlock(m_left, m_top, m_left + m_sizex - 1, m_top + m_sizey - 1, m_saveBlock);
        m_saveBlock.clear();
    }

    InputRelease();
    if ((code & K_TYPEMASK) != K_RESIZE)
        Application::getInstance().SetHelpLine();

    if(m_fMain)
    {
        if(code)
            WndManager::getInstance().PutInput(code);
        Hide(false);
        return K_CLOSE;
    }
    else
    {
        m_menu.clear();
        return code;
    }
}   

int Menu::GetNextItem(int n)
{
    int size = static_cast<int>(m_menu.size());
    for(int i = 0; i < size; ++i)
    {
        ++n;
        if(n >= size)
            n = 0;

        auto type = m_menu[n].type;
        if((type & MENU_TYPE_MASK)  == MENU_ITEM
        && (type & MENU_STATE_MASK) != MENU_DISABLED
        && !m_menu[n].name.empty())
        {
            return n;
        }
    }

    return -1;
}

int Menu::GetPrevItem(int n)
{
    int size = static_cast<int>(m_menu.size());
    for(int i = 0; i < size; ++i)
    {
        --n;
        if(n < 0)
            n = (int)(size - 1);

        auto type = m_menu[n].type;
        if((type & MENU_TYPE_MASK)  == MENU_ITEM
        && (type & MENU_STATE_MASK) != MENU_DISABLED
        && !m_menu[n].name.empty())
        {
            return n;
        }
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////
input_t LineMenu::Activate(bool capture)
{
    //LOG(DEBUG) << __FUNC__;
    if (m_menu.empty())
        return K_CLOSE;
  
    m_selected = 0;
    m_sizex = WndManager::getInstance().m_sizex - m_left;
    m_sizey = 1;

    WndManager::getInstance().GetBlock(m_left, m_top, m_left + m_sizex - 1, m_top + m_sizey - 1, m_saveBlock);

    Refresh();
    if(capture)
    {
        Show(false);
        m_fMain = true;
        InputCapture();
        input_t rc = Application::getInstance().MainProc(K_CLOSE);
        return rc;
    }
    return 0;
}

input_t LineMenu::Close(input_t code)
{
    if (m_menu.empty())
        return 0;

    return Menu::Close(code);
}

bool LineMenu::Refresh()
{
    //LOG(DEBUG) << __FUNC__;

    pos_t x = m_left;

    Application::getInstance().SetHelpLine(m_menu[m_selected].helpLine);

    WndManager::getInstance().StopPaint();
    bool rc = WndManager::getInstance().GotoXY(x, m_top);
    
    int n = 0;
    for(auto& m : m_menu)
    {
        if((m.type & MENU_TYPE_MASK) == MENU_ITEM && m.name.empty())
        {
            m.x    = 0;
            m.y    = 0;
            m.size = 0;
            continue;
        }

        m.x = x;
        m.y = m_top;

        if(n != m_selected)
            WndManager::getInstance().SetTextAttr(ColorMenu);
        else
        {
            WndManager::getInstance().SetTextAttr(ColorMenuSel);
            m_cursorx = x;
        }

        for(size_t i = 0; i < m.name.size(); ++i, ++x)
        {
            char c = m.name[i];
            if(c != '&')
                WndManager::getInstance().WriteChar(c);
            else
            {
                //& marked char
                if(n != m_selected)
                    WndManager::getInstance().SetTextAttr(ColorMenuB);
                else
                    WndManager::getInstance().SetTextAttr(ColorMenuBSel);

                WndManager::getInstance().WriteChar(m.name[++i]);
                m.iKey = std::toupper(m.name[i]);

                if(n != m_selected)
                    WndManager::getInstance().SetTextAttr(ColorMenu);
                else
                    WndManager::getInstance().SetTextAttr(ColorMenuSel);
            }
        }
        m.size = x - m.x;
        ++x;
        ++n;
        if (x >= WndManager::getInstance().m_sizex)
            break;

        WndManager::getInstance().SetTextAttr(ColorMenu);
        WndManager::getInstance().WriteChar();
    }

    if(x < WndManager::getInstance().m_sizex)
        WndManager::getInstance().FillRect(x, m_top, WndManager::getInstance().m_sizex - x, 1, ' ', ColorMenu);

    WndManager::getInstance().BeginPaint();
    rc = WndManager::getInstance().ShowBuff(m_left, m_top, m_sizex, m_sizey);

    return rc;
}

input_t LineMenu::EventProc(input_t code)
{
    bool close{false};//close menu
    int open{0};//open next frame menu
    int select = m_selected;

    if (code == K_TIME || code == K_FOCUSLOST || code == K_FOCUSSET)
    {
        return 0;
    }

    if(m_nextMenu)
    {
        code = m_nextMenu->EventProc(code);
        if(!m_nextMenu->IsActive())
        {
            m_nextMenu.reset();
            if(code == K_ESC)
            {
                return 0;
            }
            else if(code == K_LEFT || code == K_RIGHT)
            {
                open = 1;
            }
            else if(K_MOUSE == (code & K_MOUSE))
            {
                open = 1;
            }
            else
                return Close(code);
        }
    }

    if((code & K_TYPEMASK) == K_RESIZE)
        return Close(code);

    if(!m_menu.empty() && code)
    {
        LOG(DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;
        if(code == K_ESC)
        {
            close = true;
        }
        else if(code == K_ENTER || code == K_DOWN)
        {
            open = 4;
        }
        else if(code == K_LEFT)
        {
            select = GetPrevItem(select);
        }
        else if(code == K_RIGHT)
        {
            select = GetNextItem(select);
        }
        else if(code == K_HOME)
        {
            select = 0;
        }
        else if(code == K_END)
        {
            select = (int)(m_menu.size() - 1);
        }
        else if(0 == (code & K_TYPEMASK) && 0 == (code & K_CTRL) && 0 == (code & K_ALT))
        {
            char16_t wc = std::towupper((char16_t)code);
            int n = 0;
            for (const auto& m : m_menu)
            {
                if ((m.type & MENU_TYPE_MASK) == MENU_ITEM
                    && wc == m.iKey)
                {
                    //hot key
                    select = n;
                    open = 4;
                    break;
                }
                ++n;
            }
        }
        else if(K_MOUSE == (code & K_MOUSE))
        {
            pos_t x = K_GET_X(code);
            pos_t y = K_GET_Y(code);
            if(x >= m_left && x < m_left + m_sizex
                && y >= m_top  && y < m_top + m_sizey)
            {
                //LOG(DEBUG) << "LineMenu::Check mouse c=" << std::hex << code << std::dec << " x=" << x << " y=" << y;
                if((code & K_TYPEMASK) != K_MOUSEKL
                && (code & K_TYPEMASK) != K_MOUSEKR
                && (code & K_TYPEMASK) != K_MOUSEKUP)
                    return 0;

                int n = 0;
                for (const auto& m : m_menu)
                {
                    if ((m.type & MENU_TYPE_MASK) == MENU_ITEM
                        && y == m.y && x >= m.x && x < m.x + m.size)
                    {
                        if (n == select && m_nextMenu)
                            return 0;

                        if ((code & K_TYPEMASK) == K_MOUSEKL
                         || (code & K_TYPEMASK) == K_MOUSEKR)
                        {
                            select = n;
                            open = 2;
                        }
                        else
                            open = 3;
                        break;
                    }
                    ++n;
                }
            }
            else
            {
                if((code & K_TYPEMASK) == K_MOUSEKUP)
                    return Close(code);
                else
                    return 0;
            }
        }

        if(close)
            return Close(0);
        else
        {
            if(select != m_selected)
            {
                m_selected = select;
                if(m_nextMenu)
                {
                    open = 1;
                    m_nextMenu.reset();
                }
                Refresh();
            }

            if(open)
            {
                LOG(DEBUG) << "LineMenu::Select open=" << open;
                code = m_menu[m_selected].code;

                if((code & K_TYPEMASK) == K_MENU)
                {
                    if (m_nextMenu)
                    {
                        LOG(DEBUG) << "next menu close 3";
                        m_nextMenu.reset();
                    }

                    auto menu = Application::getInstance().GetMenu(m_menu[m_selected].code - K_MENU);
                    if (menu)
                    {
                        m_nextMenu = std::make_unique<FrameMenu>(*menu, m_menu[m_selected].x, (pos_t)1);
                        if (m_nextMenu)
                            m_nextMenu->Activate();
                    }
                }
                else if(open == 4 || open == 3)
                {
                    //menu by key release
                    return Close(code);
                }
            }
        }

        return 0;
    }
    else if((code & K_TYPEMASK) == K_MENU)
    {
        code = Activate(true);
        return code;
    }

    return code;
}

/////////////////////////////////////////////////////////////////////////////
input_t FrameMenu::Activate(bool capture)
{
    //LOG(DEBUG) << __FUNC__;
    if (m_menu.empty())
        return K_CLOSE;

    size_t n{};
    pos_t maxKey{}; //max key string
    for(const auto& m : m_menu)
    {
        pos_t strLen{};
        pos_t keyLen{};
        if(!m.name.empty())
        {
            auto pos = m.name.find(';');
            if(pos == std::string::npos)
            {
                strLen = (pos_t)m.name.size();
                keyLen = (pos_t)Application::getInstance().GetKeyName(m.code).size();
            }
            else
            {
                strLen = (pos_t)pos;
                keyLen = (pos_t)(m.name.size() - pos);
            }

            if (m_sizex < strLen)
                m_sizex = strLen;
            if (maxKey < keyLen)
                maxKey = keyLen;
            ++n;

            LOG(DEBUG) << "menu code=" << std::hex << m.code << std::dec 
                << " key=" << Application::getInstance().GetKeyName(m.code) << ";";
        }
    }

    if (0 == n)
        return K_CLOSE;

    m_sizex += 5 + maxKey;//really one more by simbol &
    m_sizey = static_cast<pos_t>(m_menu.size()) + 2;

    m_selected = GetNextItem(-1);

    if(m_sizex >= WndManager::getInstance().m_sizex - 1)
        m_sizex = WndManager::getInstance().m_sizex - 1;
    if(m_left + m_sizex >= WndManager::getInstance().m_sizex - 1)
        m_left = WndManager::getInstance().m_sizex - m_sizex - 1;

    if(m_sizey >= WndManager::getInstance().m_sizey - 1)
        m_sizey = WndManager::getInstance().m_sizey - 1;
    if(m_top + m_sizey >= WndManager::getInstance().m_sizey - 1)
        m_top = WndManager::getInstance().m_sizey - m_sizey - 1;

    //LOG(DEBUG) << "FrameMenu::Activate x=" << m_left << " y=" << m_top << " sx=" << m_sizex << " sy=" << m_sizey;

    WndManager::getInstance().GetBlock(m_left, m_top, m_left + m_sizex - 1, m_top + m_sizey - 1, m_saveBlock);

    Refresh();

    int shadeMode = SHADE_ALL | SHADE_PAINT | SHADE_SAVE;
    if (m_top <= 1)
        shadeMode &= ~SHADE_TOP;
    m_shade = std::make_unique<Shade>(m_left, m_top, m_sizex, m_sizey, shadeMode);

    if(capture)
        InputCapture();

    return 0;
}

bool FrameMenu::Refresh()
{
    //LOG(DEBUG) << __FUNC__;

    if(m_selected != -1)
        Application::getInstance().SetHelpLine(m_menu[m_selected].helpLine);

    WndManager::getInstance().StopPaint();

    bool rc = WndManager::getInstance().GotoXY(m_left, m_top)
        && WndManager::getInstance().SetTextAttr(ColorMenuBorder)
        && WndManager::getInstance().WriteChar(ACS_ULCORNER)
        && WndManager::getInstance().FillRect(m_left + 1, m_top, m_sizex - 2, 1, ACS_HLINE, ColorMenuBorder)
        && WndManager::getInstance().GotoXY(m_left + m_sizex - 1, m_top)
        && WndManager::getInstance().WriteChar(ACS_URCORNER);

    int n = 0;
    pos_t y = m_top + 1;
    for(auto mi = m_menu.begin(); mi != m_menu.end() && y < WndManager::getInstance().m_sizey - 1; ++mi, ++y, ++n)
    {
        if((mi->type & MENU_TYPE_MASK) == MENU_ITEM && mi->name.empty())
        {
            mi->x    = 0;
            mi->y    = 0;
            mi->size = 0;
            --y;
            continue;
        }

        mi->x = m_left + 1;
        mi->y = y;

        rc = WndManager::getInstance().GotoXY(m_left, y)
          && WndManager::getInstance().SetTextAttr(ColorMenuBorder)
          && WndManager::getInstance().WriteChar(ACS_VLINE);

        if((mi->type & MENU_TYPE_MASK) == MENU_SEPARATOR)
            rc = WndManager::getInstance().FillRect(m_left + 1, y, m_sizex - 2, 1, ACS_HLINE, ColorMenuBorder);
        else
        {
            if(mi->type & MENU_DISABLED)
                WndManager::getInstance().SetTextAttr(ColorMenuDisabled);
            else if(n != m_selected)
                WndManager::getInstance().SetTextAttr(ColorMenu);
            else
                WndManager::getInstance().SetTextAttr(ColorMenuSel);

            WndManager::getInstance().WriteChar();
            pos_t x = 1;

            const std::string& name = mi->name;
            std::string key;
            if(!name.empty())
            {
                size_t i;
                for(i = 0; i < name.size() && name[i] != ';'; ++i, ++x)
                {
                    char c = name[i];
                    if(c != '&')
                        WndManager::getInstance().WriteChar(c);
                    else
                    {
                        if(mi->type & MENU_DISABLED)
                            WndManager::getInstance().WriteChar(name[++i]);
                        else
                        {
                            if(n != m_selected)
                                WndManager::getInstance().SetTextAttr(ColorMenuB);
                            else
                                WndManager::getInstance().SetTextAttr(ColorMenuBSel);

                            WndManager::getInstance().WriteChar(name[++i]);
                            mi->iKey = std::toupper(name[i]);

                            if(n != m_selected)
                                WndManager::getInstance().SetTextAttr(ColorMenu);
                            else
                                WndManager::getInstance().SetTextAttr(ColorMenuSel);
                        }
                    }
                }

                if (i < name.size())
                    key = name.substr(i);
                else
                    key = Application::getInstance().GetKeyName(mi->code);
            }

            auto klen = key.size();
            mi->size = m_sizex - 2;

            for(; x < mi->size - (pos_t)klen - 1; ++x)
                WndManager::getInstance().WriteChar();

            auto ki = key.cbegin();
            for(; x < mi->size; ++x)
            {
                char c;
                if(ki != key.cend())
                    c = *ki++;
                else
                    c = ' ';

                if(x != mi->size - 1)
                    WndManager::getInstance().WriteChar(c);
                else
                {
                    if((mi->code & K_TYPEMASK) == K_MENU)
                        WndManager::getInstance().WriteChar('>');
                    else
                        WndManager::getInstance().WriteChar(c);
                }
            }
            WndManager::getInstance().SetTextAttr(ColorMenuBorder);
        }
        rc = WndManager::getInstance().GotoXY(m_left + m_sizex - 1, y);
        rc = WndManager::getInstance().WriteChar(ACS_VLINE);
    }

    rc = WndManager::getInstance().GotoXY(m_left, y)
      && WndManager::getInstance().SetTextAttr(ColorMenuBorder)
      && WndManager::getInstance().WriteChar(ACS_LLCORNER)
      && WndManager::getInstance().FillRect(m_left + 1, y, m_sizex - 2, 1, ACS_HLINE, ColorMenuBorder)
      && WndManager::getInstance().GotoXY(m_left + m_sizex - 1, y)
      && WndManager::getInstance().WriteChar(ACS_LRCORNER);

    _assert(m_sizey == y - m_top + 1);
    m_sizey = y - m_top + 1;//real size ???

    WndManager::getInstance().BeginPaint();
    rc = WndManager::getInstance().ShowBuff(m_left, m_top, m_sizex, m_sizey);

    return rc;
}

input_t FrameMenu::EventProc(input_t code)
{
    bool close    = false;//close menu
    int  open     = 0;//open next frame menu
    int  selected = m_selected;

    if(m_nextMenu)
    {
        code = m_nextMenu->EventProc(code);
        if(!m_nextMenu->IsActive())
        {
            LOG(DEBUG) << "next not active code=" << std::hex << code << std::dec;
            m_nextMenu.reset();
            if (code == 0)
            {
                _assert(0);//???
                return 0;
            }
            else if(code == K_ESC
                 || code == K_LEFT
                 || code == K_RIGHT)
            {
                return 0;
            }
            else if(K_MOUSE == (code & K_MOUSE))
            {
                open = 1;
            }
            else
            {
                return Close(code);
            }
        }
    }

    if((code & K_TYPEMASK) == K_RESIZE)
        return Close(code);

    if(!m_menu.empty() && code)
    {
        LOG(DEBUG) << "FrameMenu::Proc menu code=" << std::hex << code << std::dec;
        if (code == K_ESC || code == K_LEFT)
        {
            return Close(code);
        }
        else if(code == K_RIGHT)
        {
            input_t mcode = m_menu[m_selected].code;
            if ((mcode & K_TYPEMASK) == K_MENU && (mcode & K_CODEMASK) != 0)
            {
                //open next menu
                open = 4;
            }
            else
                return Close(code);
        }
        else if(code == K_ENTER)
        {
            //open next menu
            open = 4;
        }
        else if(code == K_UP)
        {
            selected= GetPrevItem(selected);
        }
        else if(code == K_DOWN)
        {
            selected = GetNextItem(selected);
        }
        else if(code == K_HOME)
        {
            selected = GetNextItem(-1);
        }
        else if(code == K_END)
        {
            selected = GetPrevItem(selected);
        }
        else if(0 == (code & K_TYPEMASK) && 0 == (code & K_CTRL) && 0 == (code & K_ALT))
        {
            char16_t wc = std::towupper((char16_t)code);
            int n = 0;
            for (auto mi = m_menu.cbegin(); mi != m_menu.cend(); ++mi, ++n)
            {
                if ((mi->type & MENU_TYPE_MASK) == MENU_ITEM
                    && (mi->type & MENU_DISABLED) == 0
                    && wc == mi->iKey)
                {
                    //hot key
                    selected = n;
                    open = 4;
                    break;
                }
            }
        }
        else if(K_MOUSE == (code & K_MOUSE))
        {
            int x = K_GET_X(code);
            int y = K_GET_Y(code);
            if(x >= m_left && x < m_left + m_sizex
            && y >= m_top  && y < m_top + m_sizey)
            {
                //LOG(DEBUG) << "FrameMenu::Check mouse c=" << std::hex << code << std::dec << " x=" << x << " y=" << y;
                if((code & K_TYPEMASK) != K_MOUSEKL
                && (code & K_TYPEMASK) != K_MOUSEKUP)
                    return 0;

                int n = 0;
                for (auto mi = m_menu.cbegin(); mi != m_menu.cend(); ++mi, ++n)
                {
                    if ((mi->type & MENU_TYPE_MASK) == MENU_ITEM
                        && (mi->type & MENU_DISABLED) == 0
                        && y == mi->y && x >= mi->x && x < mi->x + mi->size)
                    {
                        if (n == selected && m_nextMenu)
                            return 0;

                        if ((code & K_TYPEMASK) == K_MOUSEKL)
                        {
                            m_leftMKey = true;
                            selected = n;
                            open = 2;
                        }
                        else if ((code & K_TYPEMASK) == K_MOUSEKUP)
                        {
                            if (m_leftMKey)
                            {
                                m_leftMKey = false;
                                open = 3;
                            }
                        }
                        break;
                    }
                }
            }
            else
            {
                if((code & K_TYPEMASK) != K_MOUSEKUP)
                {
                    if(!m_fMain)
                        return code;
                    else
                        return Close(code);
                }
                else
                {
                    if(!m_leftMKey && m_fMain)
                        return 0;

                    m_leftMKey = false;

                    if(!m_fMain)
                        return code;
                    else
                        return Close(0);
                }
            }
        }

        if(close)
            return Close(0);
        else
        {
            if(selected != m_selected)
            {
                m_selected = selected;
                if(m_nextMenu)
                {
                    LOG(DEBUG) << "next menu close 1";
                    m_nextMenu.reset();
                    open = 1;
                }
                Refresh();
            }

            if(open && m_selected != -1)
            {
                code = m_menu[m_selected].code;

                if((code & K_TYPEMASK) == K_MENU && (code & K_CODEMASK) != 0)
                {
                    if(m_nextMenu)
                    {
                        LOG(DEBUG) << "next menu close 2";
                        m_nextMenu.reset();
                    }
                    
                    auto menu = Application::getInstance().GetMenu(m_menu[m_selected].code - K_MENU);
                    if (menu)
                    {
                        m_nextMenu = std::make_unique<FrameMenu>(*menu, 
                            static_cast<pos_t>(m_left + m_sizex - 1), static_cast<pos_t>(m_menu[m_selected].y + 1));
                        if (m_nextMenu)
                        {
                            auto rc = m_nextMenu->Activate();
                            if (rc == K_CLOSE)
                                m_nextMenu.reset();
                        }
                    }
                }
                else if(open == 4 || open == 3)
                {
                    //menu by mouse key release
                    LOG(DEBUG) << "FrameMenu::Select open" << open;
                    return Close(code);
                }
            }
        }

        return 0;
    }
    else if((code & K_TYPEMASK) == K_MENU)
    {
        Activate();
        return 0;
    }

    return code;
}

/////////////////////////////////////////////////////////////////////////////
input_t PopupMenu::Activate(bool capture)
{
    FrameMenu::Activate();
    if(capture)
    {
        Show(false);
        m_fMain = true;
        InputCapture();
        input_t rc = Application::getInstance().MainProc(K_CLOSE);
        return rc;
    }

    return 0;
}
