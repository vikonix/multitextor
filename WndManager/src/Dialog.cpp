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
#include "WndManager/Dialog.h"
#include "WndManager/DlgControls.h"
#include "WndManager/App.h"

#include <cwctype>


/////////////////////////////////////////////////////////////////////////////
namespace _WndManager
{

Dialog::Dialog(const std::list<control>& controls, pos_t x, pos_t y)
    : FrameWnd(
        x = (x != MAX_COORD) ? x : (WndManager::getInstance().m_sizex - controls.front().sizex) / 2,
        y = (y != MAX_COORD) ? y : (WndManager::getInstance().m_sizey - controls.front().sizey) / 2,
        controls.front().sizex, controls.front().sizey, BORDER_FULL)
    , m_Shade(x, y, controls.front().sizex, controls.front().sizey)
{
    //LOG(DEBUG) << "    Dialog";

    m_pColorWindow       = &ColorDialog;
    m_pColorWindowTitle  = &ColorDialogTitle;
    m_pColorWindowBorder = &ColorDialogBorder;

    if(x < 0 || y < 0 
        || WndManager::getInstance().m_sizex < controls.front().sizex 
        || WndManager::getInstance().m_sizey < controls.front().sizey)
    {
        LOG(ERROR) << __FUNC__ << "ERROR Dialog small screen size !!!";
        m_left  = 0;
        m_top   = 0;
        m_sizex = 0;
        m_sizey = 0;
        return;
    }

    size_t nRadioIndex{0};
    size_t pos{ 0 };

    for(const auto& control : controls)
    {
        switch((control.type) & CTRL_TYPE_MASK)
        {
        case 0:
            continue;

        case CTRL_TITLE:
        case CTRL_STATIC:
        case CTRL_LINE:
            m_controls.push_back(std::make_shared<CtrlStatic>(*this, control, pos));
            break;
        case CTRL_DEFBUTTON:
        case CTRL_BUTTON:
            m_controls.push_back(std::make_shared<CtrlButton>(*this, control, pos));
            break;
        case CTRL_CHECK:
            m_controls.push_back(std::make_shared<CtrlCheck>(*this, control, pos));
            break;
        case CTRL_RADIO:
            m_controls.push_back(std::make_shared<CtrlRadio>(*this, control, pos, nRadioIndex++));
            break;
        case CTRL_GROUP:
            m_controls.push_back(std::make_shared<CtrlGroup>(*this, control, pos));
            break;
        case CTRL_EDIT:
            m_controls.push_back(std::make_shared<CtrlEdit>(*this, control, pos));
            break;
        case CTRL_LIST:
            m_controls.push_back(std::make_shared<CtrlList>(*this, control, pos));
            break;
        case CTRL_DROPLIST:
            m_controls.push_back(std::make_shared<CtrlDropList>(*this, control, pos));
            break;
        case CTRL_EDITDROPLIST:
            m_controls.push_back(std::make_shared<CtrlEditDropList>(*this, control, pos));
            break;
        case CTRL_COLOR:
            m_controls.push_back(std::make_shared<CtrlColor>(*this, control, pos));
            break;

        default:
            LOG(ERROR) << __FUNC__ << "Bad dialog control " << std::hex << control.type << std::dec;
            _assert(0);
            break;
        }

        if(((control.type) & CTRL_TYPE_MASK) != CTRL_RADIO)
            nRadioIndex = 0;
        ++pos;
    }

    Select(GetNextTabItem());
}

void Dialog::UpdateVar()
{
    for(auto& ctl : m_controls)
      ctl->UpdateVar();
}

input_t Dialog::Close(int id)
{
    bool rc = OnClose(id);
    if(!rc)
        return 0;

    if(id != ID_CANCEL)
        UpdateVar();

    Hide();
    WndManager::getInstance().SetActiveView(m_activeView);
    if(!m_saveHelpLine)
        Application::getInstance().SetHelpLine();

    return K_CLOSE | id;
}

input_t Dialog::Activate()
{
    if(!m_sizex || !m_sizey)
    {
        MsgBox(MBoxKey::OK, "Dialog Box",
            { "Error !", 
            "Very small screen size." }
        );
        return ID_CANCEL;
    }

    m_activeView = WndManager::getInstance().GetActiveView();

    StopPaint();
    bool rc = OnActivate();
    BeginPaint();
    if(!rc)
        return ID_CANCEL;

    AllignButtons();
    Show();
    Application::getInstance().SetHelpLine();
    Refresh();

    InputCapture();
    input_t code = Application::getInstance().MainProc(K_CLOSE);
    return code & K_CODEMASK;
}

bool Dialog::AllignButtons()
{
    pos_t prevx = 0;
    pos_t prevy = 0;
    size_t size = m_controls.size();

    for(size_t i = 1; i <= size; ++i)
    {
        auto control = m_controls[size - i];
        int t = control->m_type;
        if((t & CTRL_HIDE) == CTRL_HIDE)
            continue;

        int type = t & CTRL_TYPE_MASK;
        if(type == CTRL_BUTTON
        || type == CTRL_DEFBUTTON)
        {
            pos_t x, y, sx, sy;
            control->GetPos(x, y, sx, sy);

            if(t & CTRL_ALIGN_RIGHT)
            {
                if(prevy != y)
                    x = m_sizex - sx - 3;
                else
                    x = prevx - sx - 1;
                control->SetPos(x);
            }

            prevx = x;
            prevy = y;
        }
    }

    return true;
}

size_t Dialog::GetNextItem()
{
    size_t n = m_selected;
    size_t size = m_controls.size();

    for(size_t i = 0; i < size; ++i)
    {
        ++n;
        if(n >= size)
            n = 0;

        int t = m_controls[n]->m_type;
        if(0 != (t & CTRL_DISABLED))
            continue;

        t &= CTRL_TYPE_MASK;

        if(t == CTRL_BUTTON
        || t == CTRL_DEFBUTTON
        || t == CTRL_CHECK
        || t == CTRL_RADIO
        || t == CTRL_EDIT
        || t == CTRL_LIST
        || t == CTRL_DROPLIST
        || t == CTRL_EDITDROPLIST
        || t == CTRL_COLOR)
            return n;
    }

    return 0;
}

size_t Dialog::GetNextTabItem()
{
    size_t n = m_selected;
    size_t size = m_controls.size();
    bool skip = (m_controls[n]->m_type & CTRL_TYPE_MASK) == CTRL_RADIO;

    for(size_t i = 0; i < size; ++i)
    {
        ++n;
        if(n >= size)
            n = 0;

        int t    = m_controls[n]->m_type;
        int type = t & CTRL_TYPE_MASK;

        if(skip && type == CTRL_RADIO)
            continue;
        else
            skip = false;

        if(0 != (t & CTRL_DISABLED))
            continue;

        if(type == CTRL_BUTTON
        || type == CTRL_DEFBUTTON
        || type == CTRL_CHECK
        || type == CTRL_RADIO
        || type == CTRL_EDIT
        || type == CTRL_LIST
        || type == CTRL_DROPLIST
        || type == CTRL_EDITDROPLIST
        || type == CTRL_COLOR)
        {
            if(type == CTRL_RADIO)
            {
                //find selected radio
                for(size_t j = n; j < size; ++j)
                {
                    if((m_controls[j]->m_type & CTRL_TYPE_MASK) != CTRL_RADIO)
                        break;

                    if(std::dynamic_pointer_cast<CtrlRadio>(m_controls[j])->m_checked)
                    {
                        n = j;
                        break;
                    }
                }
            }

            return n;
        }
    }

    return 0;
}

size_t Dialog::GetPrevItem()
{
    size_t n = m_selected;
    size_t size = m_controls.size();
    for(size_t i = 0; i < size; ++i)
    {
        --n;
        if(n == 0)
            n = size - 1;

        int t = m_controls[n]->m_type;
        if(0 != (t & CTRL_DISABLED))
            continue;

        t &= CTRL_TYPE_MASK;

        if(t == CTRL_BUTTON
        || t == CTRL_DEFBUTTON
        || t == CTRL_CHECK
        || t == CTRL_RADIO
        || t == CTRL_EDIT
        || t == CTRL_LIST
        || t == CTRL_DROPLIST
        || t == CTRL_EDITDROPLIST
        || t == CTRL_COLOR)
            return n;
    }

    return 0;
}

size_t Dialog::GetPrevTabItem()
{
    size_t n = m_selected;
    size_t size = m_controls.size();
    bool skip = (m_controls[n]->m_type & CTRL_TYPE_MASK) == CTRL_RADIO;

    for(size_t i = 0; i < size; ++i)
    {
        --n;
        if(!n)
            n = size - 1;

        int t    = m_controls[n]->m_type;
        int type = t & CTRL_TYPE_MASK;

        if(skip && type == CTRL_RADIO)
            continue;
        else
            skip = false;

        if(0 != (t & CTRL_DISABLED))
            continue;

        if(type == CTRL_BUTTON
        || type == CTRL_DEFBUTTON
        || type == CTRL_CHECK
        || type == CTRL_RADIO
        || type == CTRL_EDIT
        || type == CTRL_LIST
        || type == CTRL_DROPLIST
        || type == CTRL_EDITDROPLIST
        || type == CTRL_COLOR)
        {
            if(type == CTRL_RADIO)
            {
                //find selected radio
                for(size_t j = 0; j <= n; ++j)
                {
                    if((m_controls[n - j]->m_type & CTRL_TYPE_MASK) != CTRL_RADIO)
                            break;
                    if (std::dynamic_pointer_cast<CtrlRadio>(m_controls[j])->m_checked)
                    {
                        n -= j;
                        break;
                    }
                }
            }

            return n;
        }
    }

    return 0;
}

int Dialog::SelectItem(int id)
{
    size_t n = m_selected;

    for(const auto& control : m_controls)
    {
        if(control->m_id == id)
        {
          int t = control->m_type;
          if(t & CTRL_DISABLED)
                break;

          t &= CTRL_TYPE_MASK;

          if(t == CTRL_BUTTON
          || t == CTRL_DEFBUTTON
          || t == CTRL_CHECK
          || t == CTRL_RADIO
          || t == CTRL_EDIT
          || t == CTRL_LIST
          || t == CTRL_DROPLIST
          || t == CTRL_EDITDROPLIST
          || t == CTRL_COLOR)
          {
                Select(control->m_pos);
                return m_controls[n]->m_id;
          }
          else
                break;
        }
    }

    return 0;
}

input_t Dialog::Select(size_t n)
{
    if (n == m_selected)
        return {};

    m_controls[m_selected]->LostFocus();
    auto rc = m_controls[n]->SetFocus();
    m_selected = n;

    return rc;
}

int Dialog::GetSelectedId()
{
    return m_controls[m_selected]->m_id;
}

std::shared_ptr<Control> Dialog::GetItem(int id)
{
    for (const auto& control : m_controls)
        if(control->m_id == id)
            return control;

    return {};
}

bool Dialog::Refresh()
{
    Application::getInstance().SetHelpLine(m_controls[m_selected]->m_helpLine);
    m_Shade.Paint();

    StopPaint();
    FrameWnd::Refresh();
    _Refresh();
    BeginPaint();

    bool rc = WndManager::getInstance().ShowBuff(m_left, m_top, m_sizex, m_sizey);
    return rc;
}

bool Dialog::_Refresh()
{
    //LOG(DEBUG) << "    Dialog::_Refresh";

    Application::getInstance().SetHelpLine(m_controls[m_selected]->m_helpLine);
    StopPaint();

    for (auto& control : m_controls)
    {
        if ((control->m_type & CTRL_HIDE) != CTRL_HIDE)
            if (m_selected != control->m_pos)
                control->Refresh();
    }

    GotoXY(m_controls[m_selected]->m_posx + m_controls[m_selected]->m_dcursorx,
           m_controls[m_selected]->m_posy + m_controls[m_selected]->m_dcursory);
    m_controls[m_selected]->Refresh(CTRL_SELECTED);

    UserPaint();

    BeginPaint();
    bool rc = ShowBuff(0, 0, m_sizex - 2, m_sizey - 2);
    return rc;
}

input_t Dialog::EventProc(input_t code)
{
    if((code & K_TYPEMASK) == K_RESIZE)
    {
        return Close(ID_CANCEL);
    }

    if(code & K_MOUSE)
    {
        if((code & K_TYPEMASK) == K_MOUSEKL)
            m_mouseKey = 1;
        else if((code & K_TYPEMASK) == K_MOUSEKUP)
            m_mouseKey = 0;
    }

    if(code == K_TIME && !m_mouseKey)
        return 0;

    //LOG(DEBUG) << "    Dialog::EventProc " << std::hex << code << std::dec;
    size_t n = m_selected;

    if(code & K_MOUSE)
    {
        pos_t x = K_GET_X(code);
        pos_t y = K_GET_Y(code);
        //LOG(DEBUG) << "mouse screen x=" << x << " y=" << y;
        if(code & K_MOUSEW)
        {
            //mouse wheel
            code = m_controls[m_selected]->EventProc(code & ~K_CODEMASK);
        }
        else if(x >= m_left && x < m_left + m_sizex
             && y >= m_top  && y < m_top  + m_sizey)
        {
            ScreenToClient(x, y);
            //LOG(DEBUG) << "mouse client x=" << x << " y=" << y;

            for(auto& control : m_controls)
            {
                int t = control->m_type;
                if(0 != (t & CTRL_DISABLED))
                    continue;

                t &= CTRL_TYPE_MASK;

                if (t == CTRL_BUTTON
                 || t == CTRL_DEFBUTTON
                 || t == CTRL_CHECK
                 || t == CTRL_RADIO
                 || t == CTRL_EDIT
                 || t == CTRL_LIST
                 || t == CTRL_DROPLIST
                 || t == CTRL_EDITDROPLIST
                 || t == CTRL_COLOR)
                {
                    if (control->CheckMouse(x, y))
                    {
                        //LOG(DEBUG) << "mouse select " << control->m_pos;
                        n = control->m_pos;

                        code = control->EventProc(K_MAKE_COORD_CODE((code & ~K_CODEMASK), x, y));
                        break;
                    }
                }
            }
        }
        else
        {
            //LOG(DEBUG) << "mouse outside";
            if((code & K_TYPEMASK) == K_MOUSEKUP)
                code = K_ESC;
            else
                code = 0;
        }
    }
    else if(code == K_INSERT || code == K_F1)
        //return to APP
        return code;
    else
        code = m_controls[m_selected]->EventProc(code);

    if(code && 0 == (code & K_TYPEMASK) && 0 == (code & K_CTRL))
    {
        //LOG(DEBUG) << "symbol";
        char16_t wc = std::towupper(K_GET_CODE(code));
        
        for(auto& control: m_controls)
        {
            int t = control->m_type;
            if (0 != (t & CTRL_DISABLED))
                continue;

            if(wc == control->m_key)
            {
                //hot key
                t &= CTRL_TYPE_MASK;

                if(t == CTRL_BUTTON
                || t == CTRL_DEFBUTTON
                || t == CTRL_CHECK
                || t == CTRL_RADIO)
                {
                    n = control->m_pos;//select
                    code = control->EventProc(K_SPACE);
                }
                else if(t == CTRL_LIST || t == CTRL_COLOR)
                    n = control->m_pos;//select
                else if(t == CTRL_STATIC || t == CTRL_LINE || t == CTRL_GROUP)
                {
                    t = m_controls[control->m_pos + 1]->m_type;
                    if(0 == (t & CTRL_DISABLED))
                        n = control->m_pos + 1;//select next
                }

                break;
            }
        }
    }

    //LOG(DEBUG) << "code1=" << std::hex << code << std::dec;
    if(code == K_ESC)
        return Close(ID_CANCEL);
    else if(code == ID_OK
         || code == ID_CANCEL
         || code == ID_IGNORE)
        return Close(code);

    //LOG(DEBUG) << "code2";
    if(code == K_TAB)
        n = GetNextTabItem();
    else if(code == K_RIGHT
    ||      code == K_DOWN)
        n = GetNextItem();
    else if(code == (K_TAB | K_SHIFT))
        n = GetPrevTabItem();
    else if(code == K_LEFT
    ||      code == K_UP)
        n = GetPrevItem();

    bool refresh{false};
    if(n != m_selected)
    {
        code = Select(n);
        refresh = true;
    }

    if(code)
        code = DialogProc(code);

    if(code == K_ENTER)
        return Close(ID_OK);

    if(refresh)
        _Refresh();

    return 0;
}

std::pair<bool, size_t> Dialog::CtrlRadioSelect(size_t pos)
{
    //radio button checked
    bool uncheck{false};
    size_t count{0};

    size_t i;
    for(i = 1; i <= pos; ++i)
    {
        //looking for prev radio buttons to clear check
        int t = m_controls[pos - i]->m_type;
        if((t & CTRL_TYPE_MASK) != CTRL_RADIO)
            break;

        if(t & CTRL_DISABLED)
            continue;

        auto ctrl = std::dynamic_pointer_cast<CtrlRadio>(m_controls[pos - i]);
        ++count;
        if (ctrl->m_checked)
        {
            ctrl->m_checked = 0;
            ctrl->Refresh();
            uncheck = true;
            break;
        }
    }

    for(i = pos + 1; i < m_controls.size(); ++i)
    {
        //looking for next radio buttons to clear check
        int t = m_controls[i]->m_type;
        if((t & CTRL_TYPE_MASK) != CTRL_RADIO)
            break;

        if(0 != (t & CTRL_DISABLED))
            continue;

        auto ctrl = std::dynamic_pointer_cast<CtrlRadio>(m_controls[i]);
        ++count;
        if (ctrl->m_checked)
        {
            ctrl->m_checked = 0;
            ctrl->Refresh();
            uncheck = true;
            break;
        }
    }

    return {uncheck, count};
}

/////////////////////////////////////////////////////////////////////////////
input_t MsgBox(MBoxKey type, const std::string& title, const std::list<std::string>& message, const std::vector<std::string>& keys)
{
    size_t len{};
    for (auto& str : message)
        len = std::max(len, str.size());

    if(len > 70)
        len = 70;
    if(len < 30)
        len = 30;

    pos_t line{};
    std::list<control> box;

    box.push_back({ CTRL_TITLE, title, 0, nullptr, 1, line++, static_cast<pos_t>(len + 4), static_cast<pos_t>(message.size() + 6) });
    for (auto& str : message)
        box.push_back({ CTRL_STATIC, str, 0, nullptr, 1, line++ });
    box.push_back({ CTRL_LINE, "", 0, nullptr, 1, ++line, static_cast<pos_t>(len) });
    ++line;

    std::vector<std::string> defKeys{"OK", "Cancel", "Ignore"};
    if (keys.size() >= 1 && !keys[0].empty())
        defKeys[0] = keys[0];
    if (keys.size() >= 2 && !keys[1].empty())
        defKeys[1] = keys[1];
    if (keys.size() >= 3 && !keys[2].empty())
        defKeys[2] = keys[2];

    box.push_back({ CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, defKeys[0], ID_OK, nullptr, 1, line });
    if(type != MBoxKey::OK)
    {
        box.push_back({ CTRL_BUTTON | CTRL_ALIGN_RIGHT, defKeys[1], ID_CANCEL, nullptr, 1, line });
        if(type != MBoxKey::OK_CANCEL)
            box.push_back({ CTRL_BUTTON | CTRL_ALIGN_RIGHT, defKeys[2], ID_IGNORE, nullptr, 1, line });
    }

    if (WndManager::getInstance().m_sizex < box.front().sizex
     || WndManager::getInstance().m_sizey < box.front().sizey)
    {
        LOG(ERROR) << __FUNC__ << "ERROR Too small screen size !!!";
        return ID_OK;
    }

    Dialog Dlg(box);
    input_t ret = Dlg.Activate();
    return ret;
}

} //namespace _WndManager 
