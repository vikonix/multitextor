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
#include "DlgControls.h"
#include "App.h"
#include "ConsoleScreen.h"
#include "utils/logger.h"

#include <cwctype>


/////////////////////////////////////////////////////////////////////////////
bool Control::SetName(const std::string& name)
{
    //LOG(DEBUG) << __FUNC__ << " '" << name << "'";
    m_name = utf8::utf8to16(name);
    m_sizex = (pos_t)m_name.size() + m_addSize;
    return true;
}

bool Control::Paint(const std::u16string& wstr, int type)
{
    pos_t x = m_posx;
    pos_t y = m_posy;

    color_t color;
    if(0 != (type & CTRL_DISABLED))
        color = ColorDialogDisabled;
    else if(0 != (type & CTRL_SELECTED))
        color = ColorDialogSelect;
    else
        color = *m_dialog.m_pColorWindow;

    for(size_t i = 0; i < wstr.size(); ++i)
    {
        char16_t wc = wstr[i];
        if(wc == '&')
        {
            wc = wstr[++i];
            m_key = std::towupper(wc);
            if(0 == (type & CTRL_DISABLED))
                m_dialog.WriteWChar(x, y, wc, ColorDialogInfo);
            else
                m_dialog.WriteWChar(x, y, wc, color);
        }
        else
            m_dialog.WriteWChar(x, y, wc, color);

        if(wc == ']' || wc == ')')
        {
            if(0 != (type & CTRL_SELECTED))
                color = *m_dialog.m_pColorWindow;
        }

        ++x;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
CtrlStatic::CtrlStatic(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, nullptr, 
        control.id, control.x, control.y, control.sizex, 1, "")
{
    if(m_sizex == 0)
        m_sizex = (pos_t)m_name.size();
    else if (m_sizex < (pos_t)m_name.size())
        m_name.resize(m_sizex);
}

bool CtrlStatic::Refresh([[maybe_unused]]CtrlState state)
{
    //LOG(DEBUG) << "    CtrlStatic::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";
    color_t color;

    if((m_type & CTRL_TYPE_MASK) == CTRL_TITLE)
    {
        color = *m_dialog.m_pColorWindowTitle;
        std::string str{ ' ' + utf8::utf16to8(m_name) + ' ' };
        pos_t x = m_posx;
        if(m_posx == MAX_COORD)
            x = m_dialog.GetWSizeX() - (pos_t)m_name.size() / 2;

        m_dialog.WriteWnd(x, m_posy, str, color);
    }
    else if((m_type & CTRL_TYPE_MASK) == CTRL_LINE)
    {
        color = *m_dialog.m_pColorWindow;
        pos_t amp = 0;
        if (m_name.find('&') != std::string::npos)
            amp = 1;

        std::u16string str((size_t)m_sizex + amp, acs_t::ACS_HLINE);
        if (!m_name.empty())
            str.replace(1, m_name.size(), m_name);
        Paint(str, m_type);
    }
    else
    {
        if((m_type & CTRL_STATE_MASK) == CTRL_NORMAL)
        {
            Paint(m_name, CTRL_NORMAL);
        }
        else if((m_type & CTRL_STATE_MASK) == CTRL_NOCOLOR)
        {
            color = *m_dialog.m_pColorWindow;
            m_dialog.WriteWStr(m_posx, m_posy, m_name, color);
        }
        else
        {
            color = ColorDialogDisabled;
            m_dialog.WriteWStr(m_posx, m_posy, m_name, color);
        }
    }

    return true;
}

bool CtrlStatic::SetName(const std::string& name)
{
    pos_t sizex = m_sizex;
    Control::SetName(name);
    m_sizex = sizex;

    if((m_type & CTRL_TYPE_MASK) != CTRL_TITLE)
    {
        m_name.resize(m_sizex, ' ');
        Refresh();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
CtrlButton::CtrlButton(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, 1, 1, control.helpLine)
{
    m_sizex = (pos_t)m_name.size();
    if(m_name.find('&') != std::string::npos)
        --m_sizex;

    if((m_type & CTRL_TYPE_MASK) == CTRL_BUTTON)
    {
        m_dcursorx = 2;
        m_sizex += m_addSize = 4;

    }
    else
    {
        m_dcursorx = 3;
        m_sizex += m_addSize = 6;
    }
}


bool CtrlButton::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlButton::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";
    std::u16string name;
    if((m_type & CTRL_TYPE_MASK) == CTRL_BUTTON)
        name = u"[ " + m_name + u" ]";
    else
        name = u"[_ " + m_name + u" _]";

    Paint(name, state | (m_type & CTRL_STATE_MASK));

    return true;
}


input_t CtrlButton::EventProc(input_t code)
{
    if(code == K_SPACE || code == K_ENTER)
    {
        return m_id;
    }
    else if(code & K_MOUSE)
    {
        if((code & K_TYPEMASK) == K_MOUSEKUP)
            return m_id;
    }

    return code;
}

/////////////////////////////////////////////////////////////////////////////
CtrlCheck::CtrlCheck(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, 1, 1, control.helpLine)
{
    m_dcursorx = 1;
    m_addSize = 4;
    m_sizex = (pos_t)m_name.size() + m_addSize;

    if(m_var.has_value())
        m_checked = *std::any_cast<bool*>(m_var);
}

bool CtrlCheck::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlCheck::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";
    std::u16string name;
    
    if(!m_checked)
        name = u"[ ] " + m_name;
    else
        name = u"[x] " + m_name;

    Paint(name, state | (m_type & CTRL_STATE_MASK));

    return true;
}

input_t CtrlCheck::EventProc(input_t code)
{
    if(code == K_SPACE || code == K_ENTER)
    {
        m_checked = !m_checked;
        Refresh(CTRL_SELECTED);
        return 0;
    }
    else if(code & K_MOUSE)
    {
        if((code & K_TYPEMASK) == K_MOUSEKUP)
        {
          m_checked = !m_checked;
          Refresh(CTRL_SELECTED);
          return 0;
        }
        return 0;
    }

    return code;
}

bool CtrlCheck::UpdateVar()
{
    if (m_var.has_value())
    {
        auto var = std::any_cast<bool*>(m_var);
        if(var)
            *var = m_checked;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
CtrlRadio::CtrlRadio(Dialog& dialog, const control& control, size_t pos, size_t index)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, 1, 1, control.helpLine)
    , m_index{index}
{
    m_dcursorx = 1;
    m_addSize = 4;
    m_sizex = (pos_t)m_name.size() + m_addSize;

    if(m_var.has_value())
    {
        if(*std::any_cast<size_t*>(m_var) == m_index)//???
            m_checked = true;
        else
            m_checked = false;
    }
    else if(!m_index)
        m_checked = true;
    else
        m_checked = false;
}

bool CtrlRadio::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlRadio::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";
    std::u16string name;
    if(!m_checked)
        name = u"( ) " + m_name;
    else
        name = u"(*) " + m_name;

    Paint(name, state | (m_type & CTRL_STATE_MASK));

    return 0;
}

input_t CtrlRadio::EventProc(input_t code)
{
    bool check{false};

    if(code == K_SPACE)
        check = true;
    else if (code & K_MOUSE)
    {
        if ((code & K_TYPEMASK) == K_MOUSEKUP)
            check = true;
    }

    if(check)
    {
        SetCheck();
        return 0;
    }
    else
        return code;
}

bool CtrlRadio::UpdateVar()
{
    if (m_var.has_value() && m_checked)
    {
        auto var = std::any_cast<size_t*>(m_var);
        if (var)
            *var = m_index;
    }
    return true;
}

bool CtrlRadio::SetCheck()
{
    bool chk = m_checked;
    bool select = m_dialog.CtrlRadioSelect(m_pos);
    if(select)
        m_checked = 1;
    else
    {
        //alone radio button
        m_checked = !chk;
    }

    Refresh(CTRL_SELECTED);
    return m_checked;
}

/////////////////////////////////////////////////////////////////////////////
CtrlGroup::CtrlGroup(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, control.sizex, control.sizey, control.helpLine)
{
}

bool CtrlGroup::Refresh(CtrlState)
{
    //LOG(DEBUG) << "    CtrlGroup::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";
    color_t color = *m_dialog.m_pColorWindow;

    size_t size = m_sizex;
    if(m_name.find('&') != std::string::npos)
        ++size;

    std::u16string buff;
    buff = (char16_t)ACS_ULCORNER + m_name;
    buff.resize(size - 1, (char16_t)ACS_HLINE);
    buff += (char16_t)ACS_URCORNER;
    Paint(buff, 0);

    m_dialog.FillRect(m_posx,               m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);
    m_dialog.FillRect(m_posx - 1 + m_sizex, m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);

    buff = (char16_t)ACS_LLCORNER;
    buff.resize((size_t)m_sizex - 1, (char16_t)ACS_HLINE);
    buff += (char16_t)ACS_LRCORNER;
    m_dialog.WriteWStr(m_posx, m_posy + m_sizey - 1, buff, color);

    return true;
}

/////////////////////////////////////////////////////////////////////////////
CtrlEdit::CtrlEdit(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, control.sizex, 1, control.helpLine)
{
    if(m_var.has_value())
    {
        auto str = std::any_cast<std::string*>(m_var);
        m_name = utf8::utf8to16(*str);
    }
}

bool CtrlEdit::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlEdit::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";

    color_t color;
    if(0 != (m_type & CTRL_DISABLED))
        color = ColorDialogDisabled;
    else if(0 != (state & CTRL_SELECTED))
    {
        if(m_selected)
            color = ColorDialogFieldSel;
        else
            color = ColorDialogSelect;
    }
    else
        color = ColorDialogFieldAct;

    std::u16string str;
    if(m_offset < m_name.size())
        str = m_name.substr(m_offset);
    str.resize(m_sizex, ' ');
    m_dialog.WriteWStr(m_posx, m_posy, str, color);

    return true;
}

bool CtrlEdit::Unselect(bool del)
{
    if(m_selected)
    {
        m_selected = false;
        if(!del)
        {
            //unselect
            Refresh(CTRL_SELECTED);
        }
        else
        {
            //unselect and delete
            m_offset = 0;
            m_dcursorx = 0;
            m_name.erase();

            m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
            Refresh(CTRL_SELECTED);
        }
    }
    return true;
}

input_t CtrlEdit::EventProc(input_t code)
{
    size_t offset = m_offset;
    pos_t x = m_dcursorx;
    size_t len = m_name.size();
    bool changed{false};

    //moving
    if(code == K_LEFT)
    {
        Unselect();
        if (x)
            --x;
        else if (m_offset)
            --m_offset;
    }
    else if(code == K_RIGHT)
    {
        if(m_selected)
            Unselect();
        else
        {
            if (x < m_sizex - 1)
                ++x;
            else
                ++m_offset;
        }
    }
    else if(code == K_HOME)
    {
        Unselect();
        m_offset = 0;
        x = 0;
    }
    else if(code == K_END)
    {
        Unselect();
        if (len < (size_t)m_sizex)
        {
            m_offset = 0;
            x = (pos_t)len;
        }
        else
        {
            x = m_sizex - 1;
            m_offset = len - x;
        }
    }
    else if (code & K_MOUSE)
    {
        if ((code & K_TYPEMASK) == K_MOUSEKL)
        {
            Unselect();
            x = K_GET_X(code) - m_posx;
        }
    }
    //editing
    else if(code == K_BS)
    {
        Unselect(true);
        x = m_dcursorx;
        if(m_offset + x > 0)
        {
            if (x)
                --x;
            else if (m_offset)
                --m_offset;
            if (m_offset + x < len)
            {
                //del char at x
                m_name.erase(m_offset + x, 1);
                changed = true;
            }
        }
    }
    else if(code == K_DELETE)
    {
        Unselect(true);
        x = m_dcursorx;
        if(len && m_offset + x < len)
        {
            //del char at x
            m_name.erase(m_offset + x, 1);
            changed = true;
        }
    }
    else if((code & K_TYPEMASK) == K_SYMBOL)
    {
        if(((code & K_MODMASK) & ~K_SHIFT) == 0
        && (code & K_CODEMASK) >= K_SPACE)
        {
            code &= ~K_SHIFT;
            Unselect(true);
            x = m_dcursorx;

            char16_t wc = K_GET_CODE(code);
            //edit symbol
            if (m_offset + x > len)
                m_name.append(m_offset + x - len, ' ');
            if (Application::getInstance().IsInsertMode())
                m_name.insert(m_offset + x, 1, wc);
            else if (m_offset + x < len)
                m_name[m_offset + x] = wc;
            else
                m_name.append(1, wc);

            if (x < m_sizex - 1)
                ++x;
            else
                ++m_offset;
            changed = true;
        }
        else
            return code;
    }
/* ???
    else if(code == (K_INSERT | K_SHIFT)
        || code == ('V' | K_CTRL))
    {
        LOG(DEBUG) << "     Paste";
        x = Unselect(true);

        ???PasteFromClipboard(pBuff, l)
        //convert string
        for(int i = 0; i < l; ++i)
        {
            unsigned char c = (unsigned char) pBuff[i];
            if(c >= ' ')
                pBuff1[j++] = c;
            else if(c == 0x9)//tab
                pBuff1[j++] = ' ';
            else
                break;
        }
        f = 1;
    }
*/
    else
        return code;

    if(x != m_dcursorx)
    {
        m_dcursorx = x;
        m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
    }
    
    if (changed || offset != m_offset)
    {
        Refresh(CTRL_SELECTED);
    }

    //TPRINT(("x=%d len=%d s=%d\n", m_nPos, m_nLen, m_nBuffLen));
    return 0;
}

bool CtrlEdit::UpdateVar()
{
    if(m_var.has_value())
    {
        auto var = std::any_cast<std::string*>(m_var);
        if (var)
        {
            *var = utf8::utf16to8(m_name);
        }
    }

    return true;
}

bool CtrlEdit::SetName(const std::string& name)
{
    m_name = utf8::utf8to16(name);
    m_offset = 0;

    m_dcursorx = (pos_t)m_name.size();
    if (m_dcursorx >= m_sizex)
    {
        m_offset = (size_t)m_dcursorx - m_sizex;
        m_dcursorx = m_sizex;
    }

    Refresh();

    return true;
}

input_t CtrlEdit::SetFocus()
{
    m_selected = true;
    m_dcursorx = (pos_t)m_name.size() < m_sizex ? (pos_t)m_name.size() : m_sizex - 1;

    m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
    return K_SELECT;
}

/////////////////////////////////////////////////////////////////////////////
CtrlList::CtrlList(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, control.sizex, control.sizey, control.helpLine)
{
  m_dcursorx   = 1;
  m_dcursory   = 1;
}

bool CtrlList::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlList::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";
    m_dialog.StopPaint();
    
    color_t color;
    if(m_type & CTRL_DISABLED)
        color = ColorDialogDisabled;
    else
        color = *m_dialog.m_pColorWindow;

    size_t size = m_sizex;
    if (m_name.find('&') != std::string::npos)
        ++size;

    std::u16string buff;
    buff = (char16_t)ACS_ULCORNER + m_name;
    buff.resize(size - 1, (char16_t)ACS_HLINE);
    buff += (char16_t)ACS_URCORNER;
    Paint(buff, m_type);

    pos_t sliderPos = -1;
    if(m_sizey - 2 > 3)
    {
        size_t listSize = m_list.size();
        if(listSize >= (size_t)m_sizey - 2)
            sliderPos = (pos_t)(GetSelected() * ((size_t)m_sizey - 2) / listSize);
    }

    for(pos_t y = 0; y < m_sizey - 2; ++y)
    {
        m_dialog.WriteChar(m_posx, m_posy + 1 + y, ACS_VLINE, color);

        auto line = GetStr((size_t)m_firstLine + y);

        std::u16string wstr = utf8::utf8to16(std::string(line));
        if (wstr.size() < (size_t)m_sizex - 2)
        {
            wstr.resize((size_t)m_sizex - 2, ' ');
        }
        else
        {
            //cut line
            wstr.resize((size_t)m_sizex - 3);
            wstr += '~';
        }

        color_t lineColor = ColorDialogField;
        if(m_type & CTRL_DISABLED)
            lineColor = ColorDialogDisabled;
        else if((size_t)y == m_selected)
        {
            if(state & CTRL_SELECTED)
                lineColor = ColorDialogSelect;
            else
                lineColor = ColorDialogFieldAct;
        }
        
        m_dialog.WriteWStr(m_posx + 1, m_posy + 1 + y, wstr, lineColor);

        if(y != sliderPos)
            m_dialog.WriteChar(m_posx + m_sizex - 1, m_posy + 1 + y, ACS_VLINE, color);
        else
            //current pos in list
            m_dialog.WriteChar(m_posx + m_sizex - 1, m_posy + 1 + y, '+', color);
    }

    buff = (char16_t)ACS_LLCORNER;
    buff.resize((size_t)m_sizex - 1, (char16_t)ACS_HLINE);
    buff += (char16_t)ACS_LRCORNER;
    m_dialog.WriteWStr(m_posx, m_posy + m_sizey - 1, buff, color);

    m_dialog.BeginPaint();
    m_dialog.ShowBuff(m_posx, m_posy, m_sizex, m_sizey);

    return true;
}

bool CtrlList::UpdateVar()
{
    if (m_var.has_value())
    {
        auto var = std::any_cast<size_t*>(m_var);
        if(var)
            *var = m_firstLine + m_selected;
    }
    return true;
}

size_t CtrlList::SetSelect(size_t pos, bool refresh)
{
    size_t size = m_list.size();
    if (pos > size)
        pos = size;
    size_t sizey = (size_t)m_sizey - 3;

    if (pos != m_firstLine + m_selected)
    {
        if (pos < sizey)
            m_firstLine = 0;
        else if (pos >= size - sizey)
            m_firstLine = size - sizey - 1;
        else
            m_firstLine = pos - sizey / 2;

        m_selected = pos - m_firstLine;
        m_dcursory = (pos_t)(m_selected + 1);

        if (refresh)
        {
            m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
            Refresh();
        }
    }

    return pos;
}

input_t CtrlList::EventProc(input_t code)
{
    int n = (int)(m_firstLine + m_selected);//can be negative
    size_t size = m_list.size();
    if (size == 0)
        return code;

    size_t sizey = (size_t)m_sizey - 3;
    size_t step = 0;

    if(code == K_TIME)
    {
        code = m_mouseCmd;
    }

    if((code & K_TYPEMASK) == K_SYMBOL && code != K_BS)
    {
        if((code & K_MODMASK) == 0 && K_GET_CODE(code) > K_SPACE)
        {
/* ???       
            int f = -1;
            char buff[2] = {wchar2char(g_textCP, wchar(code & K_CODEMASK)), 0};
            f = m_pList->FindSorted(buff, g_textCP, 0);
            if(f != -1)
                n = f;
*/
        }
        else
            return code;
    }
    else if (code == K_UP)
    {
        step = 1;
        --n;
    }
    else if (code == K_DOWN)
    {
        step = 1;
        ++n;
    }
    else if(code == K_PAGEUP)
    {
        step = sizey > 0 ? sizey : 1;
        n -= (int)step;
    }
    else if(code == K_PAGEDN)
    {
        step = sizey > 0 ? sizey : 1;
        n += (int)step;
    }
    else if(code == K_HOME || code == K_HOME + K_CTRL)
        n = 0;
    else if(code == K_END || code == K_END + K_CTRL)
        n = (int)(size - 1);
    else if(code & K_MOUSE)
    {
        if((code & K_TYPEMASK) == K_MOUSEKUP)
        {
            code = 0;
            if(m_mouse2)
            {
                m_mouse2 = false;
                return K_ENTER;
            }
        }

        if((code & K_TYPEMASK) == K_MOUSEKL)
        {
            int p = K_GET_Y(code) - m_posy;
            if(p <= 0)
                m_mouseCmd = K_MOUSEWUP;
            else if(p >= m_sizey - 1)
                m_mouseCmd = K_MOUSEWDN;
            else
                m_mouseCmd = 0;

            n = (int)m_firstLine + p - 1;
            if(code & K_MOUSE2 && !m_mouseCmd)
            {
                m_mouse2 = true;
                return 0;
            }
        }
        else if((code & K_TYPEMASK) == K_MOUSEWUP)
        {
            step = sizey / 3 > 0 ? sizey / 3 : 1;
            n -= (int)step;
        }
        else if((code & K_TYPEMASK) == K_MOUSEWDN)
        {
            step = sizey / 3 > 0 ? sizey / 3 : 1;
            n += (int)step;
        }
    }
    else if(code == K_SPACE)
        return 0;
    else
        return code;

    size_t pos;
    if (n < 0)
        pos = 0;
    else if (n >= (int)size)
        pos = size - 1;
    else
        pos = (size_t)n;

    if(pos != m_firstLine + m_selected)
    {
        if (step == 0)
        {
            if (pos < m_firstLine || pos > m_firstLine + sizey)
            {
                if (pos < sizey)
                    m_firstLine = 0;
                else if (pos >= size - sizey)
                    m_firstLine = size - sizey - 1;
                else
                    m_firstLine = pos - sizey / 2;
            }
        }
        else if (pos > m_firstLine + sizey)
        {
            if (m_firstLine + step < size - sizey)
                m_firstLine += step;
            else
                m_firstLine = size - sizey - 1;
        }
        else if (pos < m_firstLine)
        {
            if (m_firstLine > step)
                m_firstLine -= step;
            else
                m_firstLine = 0;
        }

        m_selected = pos - m_firstLine;
        m_dcursory = (pos_t)(m_selected + 1);

        m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
        Refresh(CTRL_SELECTED);

        _assert(pos < 0xffffff);
        return K_SELECT + (input_t)(pos & 0xffffff);
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
CtrlDropList::CtrlDropList(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, control.sizex, control.sizey, control.helpLine)
    , m_list(dialog, control, pos)
{
    m_list.m_posy  += 1;
    m_list.m_sizey -= 1;
}

input_t CtrlDropList::SetFocus()
{
    m_dcursorx = m_sizex - 2;
    m_dcursory = 0;

    return K_SELECT;
}

bool CtrlDropList::LostFocus()
{
    if (m_listOpened)
    {
        m_listOpened = false;
        SetSelect(GetSelected());
        SetFocus();
        m_dialog.Refresh();
    }

    return true;
}

size_t CtrlDropList::SetSelect(size_t n)
{
    m_list.SetSelect(n, false);
    Refresh();
    return 0;
}

input_t CtrlDropList::EventProc(input_t code)
{
    if (code & K_MOUSE)
    {
        if ((code & K_TYPEMASK) == K_MOUSEKUP)
        {
            pos_t x = K_GET_X(code);
            pos_t y = K_GET_Y(code);

            if (!m_listOpened)
            {
                if (y == m_posy && x >= m_posx && x < m_posx + m_sizex)
                    code = K_DOWN;
            }
            else
            {
                if (y == m_posy)
                    code = K_ESC;
            }
        }
        else if (!m_listOpened)
            code = 0;
    }

    if(m_listOpened)
        code = m_list.EventProc(code);

    if(!m_listOpened && (code == K_DOWN || code == K_ENTER || code == K_SPACE))
    {
        m_listOpened = true;
        m_dcursorx = m_list.m_dcursorx;
        m_dcursory = m_list.m_dcursory;
        m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory + 1);
        m_list.Refresh(CTRL_SELECTED);
        return 0;
    }

    if(m_listOpened)
    {
        if(code == K_ENTER)
        {
            LostFocus();
            return 0;
        }

        if(code == K_ESC
        || code == K_TAB
        || code == (K_TAB | K_SHIFT)
        || code == K_LEFT
        || code == K_RIGHT
        )
        {
            LostFocus();
            return 0;
        }
    }

    return code;
}

bool CtrlDropList::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlDropList::Refresh pos=" << m_pos;

    auto n = GetSelected();
    std::u16string buff{ utf8::utf8to16(std::string(GetStr(n))) };

    buff.resize((size_t)m_sizex - 3, ' ');

    color_t color;
    if(0 != (state & CTRL_SELECTED))
        color = ColorDialogSelect;
    else if(0 != (m_type & CTRL_DISABLED))
        color = ColorDialogDisabled;
    else
        color = ColorDialogField;

    if(0 != (m_type & CTRL_DISABLED))
        m_dialog.WriteWStr(m_posx, m_posy, buff, color);
    else
        m_dialog.WriteWStr(m_posx, m_posy, buff, ColorDialogFieldAct);

    m_dialog.WriteStr(m_posx + m_sizex - 3, m_posy, "[v]", color);

    if(m_listOpened)
        m_list.Refresh(state);

    return true;
}

bool CtrlDropList::CheckMouse(pos_t x, pos_t y)
{
    if(!m_listOpened)
        return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + 1);
    else
        return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + m_sizey);
}


bool CtrlDropList::SetPos(pos_t x, pos_t y, pos_t sizex, pos_t sizey)
{
    if(x != MAX_COORD)
    {
        m_posx        = x;
        m_list.m_posx = x;
    }

    if(y != MAX_COORD)
    {
        m_posy        = y;
        m_list.m_posy = y + 1;
    }

    if(sizex > 0)
    {
        m_sizex        = sizex;
        m_list.m_sizex = sizex;
    }

    if(sizey > 0)
    {
        m_sizey        = sizey;
        m_list.m_sizey = sizey - 1;
    }

    return true;
}

std::string CtrlDropList::GetName()
{
    auto n = m_list.GetSelected();
    auto str = m_list.GetStr(n);

    return std::string(str);
}

/////////////////////////////////////////////////////////////////////////////
CtrlEditDropList::CtrlEditDropList(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, control.sizex, control.sizey, control.helpLine)
    , m_edit(dialog, control, pos)
    , m_list(dialog, control, pos)
{
    m_edit.m_sizex -= 3;

    m_list.m_sizey -= 1;
    if(m_posy + m_sizey < dialog.GetWSizeY())
        m_list.m_posy += 1;
    else
        m_list.m_posy -= m_sizey - 1;

/* ???  
  if(pControl->pName && *((long*)pControl->pName) == STR_SIGN)
  {
    //TPRINT(("SaveStr buff\n"));
    m_pSave = (StrSaveList*)pControl->pName;
    for(size_t i = 0; i < STR_SAVE; ++i)
    {
      char* pStr = m_pSave->GetStr(i);
      if(!pStr)
        break;
      m_List.AppendStr(pStr);
    }
  }
  else
    m_pSave = NULL;
*/
}

input_t CtrlEditDropList::SetFocus()
{
    auto rc = m_edit.SetFocus();
    m_dcursorx = m_edit.m_dcursorx;
    m_dcursory = m_edit.m_dcursory;

    return rc;
}

bool CtrlEditDropList::LostFocus()
{
    if (m_listOpened)
    {
        m_listOpened = false;
        SetFocus();
        m_edit.m_selected = false;
        m_dialog.Refresh();
    }

    return true;
}

input_t CtrlEditDropList::EventProc(input_t code)
{
    if((code & K_TYPEMASK) == K_MOUSEKUP)
    {
        pos_t x = K_GET_X(code);
        pos_t y = K_GET_Y(code);

        if(!m_listOpened)
            if(y == m_posy && x >= m_posx + m_sizex - 3 && x < m_posx + m_sizex)
                code = K_DOWN;

        if(m_listOpened)
            if(y == m_posy)
                code = K_ESC;
    }

    if(!m_listOpened)
        code = m_edit.EventProc(code);
    else
        code = m_list.EventProc(code);


    if(!m_listOpened && code == K_DOWN)
    {
        m_listOpened = true;
        m_dcursorx = m_list.m_dcursorx;
        m_dcursory = m_list.m_dcursory + m_list.m_posy - m_posy - 1;
        m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory + 1);
        m_list.Refresh(CTRL_SELECTED);
        return 0;
    }

    if(m_listOpened)
    {
        if(code == K_ENTER)
        {
            auto n = GetSelected();
            std::string str{ GetStr(n) };
            m_edit.SetName(str);
        }

        if(code == K_ESC
        || code == K_TAB
        || code == (K_TAB | K_SHIFT)
        || code == K_LEFT
        || code == K_RIGHT
        || code == K_ENTER
        )
        {
            LostFocus();
            if(code == K_ENTER)
                return K_SELECT | K_ENTER;
            else
                return 0;
        }
        else
            code = 0;
    }

    return code;
}

bool CtrlEditDropList::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlEditDropList::Refresh pos=" << m_pos;

    m_edit.Refresh(state);

    color_t color;
    if(0 != (state & CTRL_SELECTED))
        color = ColorDialogSelect;
    else if(0 != (m_type & CTRL_DISABLED))
        color = ColorDialogDisabled;
    else
        color = ColorDialogField;

    m_dialog.WriteStr(m_posx + m_sizex - 3, m_posy, "[v]", color);

    if(m_listOpened)
        m_list.Refresh(state);

    return true;
}

bool CtrlEditDropList::CheckMouse(pos_t x, pos_t y)
{
    if(!m_listOpened)
        return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + 1);
    else
        return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + m_sizey);
}

bool CtrlEditDropList::SetPos(pos_t x, pos_t y, pos_t sizex, pos_t sizey)
{
    if(x != MAX_COORD)
    {
        m_posx        = x;
        m_edit.m_posx = x;
        m_list.m_posx = x;
    }

    if(y != MAX_COORD)
    {
        m_posy        = y;
        m_edit.m_posy = y;
        m_list.m_posy = y + 1;
    }

    if(sizex > 0)
    {
        m_sizex           = sizex;
        m_edit.m_sizex    = sizex - 3;
        m_list.m_sizex    = sizex;
    }

    if(sizey > 0)
    {
        m_sizey        = sizey;
        m_list.m_sizey = sizey - 1;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
CtrlColor::CtrlColor(Dialog& dialog, const control& control, size_t pos)
    : Control(dialog, pos, control.type, control.name, control.var, control.id
        , control.x, control.y, 18, 6, control.helpLine)
{
    if (m_var.has_value())
    {
        auto var = std::any_cast<color_t*>(m_var);
        m_color = *var;
    }
    SetCursor();
}

color_t CtrlColor::SetVar(color_t c)
{
    if(c != m_color)
    {
        PaintSelect(0);
        m_color   = c % m_maxColor;
        m_dcursorx = 1 + (m_color % 4) * 4;
        m_dcursory = 1 +  m_color / 4;
        PaintSelect();
    }
    return m_color;
}

bool CtrlColor::UpdateVar()
{
    if (m_var.has_value())
    {
        auto var = std::any_cast<color_t*>(m_var);
        if (var)
            *var = m_color;
    }
    return true;
}

bool CtrlColor::SetCursor()
{
    m_color  %= m_maxColor;

    m_dcursorx = 1 + (m_color % 4) * 4;
    m_dcursory = 1 + m_color / 4;
    m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
    return 0;
}

bool CtrlColor::PaintSelect(bool visible, bool selected)
{
    color_t color;
    if(selected)
        color = ColorDialogSelect;
    else if(0 != (m_type & CTRL_DISABLED))
        color = ColorDialogDisabled;
    else
        color = *m_dialog.m_pColorWindow;

    char c{' '};
    if(visible)
        c = '>';

    bool rc = m_dialog.WriteChar(m_posx + m_dcursorx, m_posy + m_dcursory, c, color);
    return rc;
}

bool CtrlColor::Refresh(CtrlState state)
{
    //LOG(DEBUG) << "    CtrlColor::Refresh pos=" << m_pos;// << " name='" << utf8::utf16to8(m_name) << "'";

    color_t color;
    if(0 != (m_type & CTRL_DISABLED))
        color = ColorDialogDisabled;
    else
        color = *m_dialog.m_pColorWindow;

    size_t size = m_sizex;
    if(m_name.find('&') != std::string::npos)
        ++size;
    
    std::u16string buff;

    buff = (char16_t)ACS_ULCORNER + m_name;
    buff.resize(size - 1, ACS_HLINE);
    buff += (char16_t)ACS_URCORNER;
    Paint(buff, 0);

    m_dialog.FillRect(m_posx,               m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);
    m_dialog.FillRect(m_posx - 1 + m_sizex, m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);

    for(pos_t i = 0; i < m_maxColor / 4; ++i)
        for(pos_t j = 0; j < 4; ++j)
            m_dialog.FillRect(m_posx + 2 + j * 4, m_posy + 1 + i, 3, 1, ACS_SQUARE, (i * 4 + j) | *m_dialog.m_pColorWindow);

    PaintSelect(true, 0 != (state & CTRL_SELECTED));

    buff = (char16_t)ACS_LLCORNER;
    buff.resize((size_t)m_sizex - 1, ACS_HLINE);
    buff += (char16_t)ACS_LRCORNER;
    m_dialog.WriteWStr(m_posx, m_posy + m_sizey - 1, buff, color);

    return true;
}

input_t CtrlColor::EventProc(input_t code)
{
    color_t color = m_color;

    if(code == K_SPACE)
        return 0;
    else if(code == K_LEFT)
    {
        if (color > 0)
            color = --color % m_maxColor;
        else
            color = m_maxColor - 1;
    }
    else if(code == K_RIGHT)
    {
        if(++color >= m_maxColor)
            color = 0;
    }
    else if(code == K_UP)
    {
        color_t c = color / 4;
        if (c > 0)
            c = --c % (m_maxColor / 4);
        else
            c = (m_maxColor / 4) - 1;
        color = c * 4 + m_color % 4;
    }
    else if(code == K_DOWN)
    {
        color_t c = color / 4;
        if(++c >= m_maxColor / 4)
            c = 0;
        color = c * 4 + m_color % 4;
    }
    else if(code == K_PAGEUP || code == K_HOME)
        color = 0;
    else if(code == K_PAGEDN || code == K_END)
        color = m_maxColor - 1;
    else if(code & K_MOUSE)
    {
        if((code & K_TYPEMASK) == K_MOUSEKUP)
        {
            pos_t x = K_GET_X(code) - m_posx - 1;
            pos_t y = K_GET_Y(code) - m_posy - 1;
            if(x >= 0 && x < m_sizex - 2 && y >= 0 && y < m_sizey - 2)
                color = x / 4 + y * 4;
        }
    }

    if(color != m_color)
    {
        PaintSelect(false);
        m_color = color;
        SetCursor();
        PaintSelect(true, true);
        return K_SELECT + color + 1;
    }

    return code;
}
