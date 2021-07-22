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
#include "WndManager/App.h"

#include <ctime>
#include <iomanip>


//////////////////////////////////////////////////////////////////////////////
namespace _WndManager
{

bool Application::Init()
{
    LOG(DEBUG) << " A::Init";
    
    if (m_inited)
        return true;

    bool rc = LoadCfg();
    rc = m_wndManager.Init();
    if (rc)
        m_inited = true;
    return rc;
}

void Application::Deinit()
{
    LOG(DEBUG) << " A::Deinit";
    if (!m_inited)
        return;

    m_inited = false;
    while (m_capturedInput)
    {
        m_capturedInput->EventProc(K_ESC);
    }
    m_mainMenu.reset();
    m_wndManager.Deinit();
}

bool Application::SetMenu(const std::vector<menu_list>& menu)
{ 
    m_menuArray = menu; 
    if (!menu.empty())
        m_mainMenu = std::make_shared<LineMenu>(menu[0]);
    else
        m_mainMenu.reset();

    return true; 
}

std::optional<std::reference_wrapper<const menu_list>> Application::GetMenu(size_t n)
{ 
    if (n < m_menuArray.size())
        return std::cref(m_menuArray[n]);
    else
        return std::nullopt;
}

bool Application::SetStatusLine(const sline_list& line)
{
    m_sLine = line;

    m_wndManager.m_bottomLines = 0;
    if (!m_accessMenu.empty())
        ++m_wndManager.m_bottomLines;
    if (!m_sLine.empty())
        ++m_wndManager.m_bottomLines;

    bool rc = m_wndManager.CalcView();
    return rc;
}

menu_list Application::SetAccessMenu(const menu_list& menu)
{
    menu_list prevMenu = std::move(m_accessMenu);
    m_accessMenu = menu;

    m_wndManager.m_bottomLines = 0;
    if (!m_accessMenu.empty())
        ++m_wndManager.m_bottomLines;
    if (!m_sLine.empty())
        ++m_wndManager.m_bottomLines;
    
    m_wndManager.CalcView();

    return prevMenu;
}

bool Application::ChangeStatusLine(size_t n, std::optional<const std::string> text, stat_color color)
{
    if (m_sLine.empty())
        return true;

    std::string msg = text.has_value() ? text.value() : "";
    //LOG(DEBUG) << " A::ChangeStatusLine n=" << n << " '" << msg << "' c=" << static_cast<int>(color);

    if (n >= m_sLine.size())
    {
        LOG(ERROR) << __FUNC__ << " error n=" << n << " > size=" << m_sLine.size();
        return false;
    }

    if (n == 0 && m_sLine[0].color == stat_color::error && text.has_value() && color != stat_color::error)
        //save error line until input event
        return true;

    if (m_sLine[n].text.empty() && !text.has_value())
        return true;

    if (m_sLine[n].text != msg || m_sLine[n].color != color)
    {
        m_sLine[n].text = msg;
        m_sLine[n].color = color;
        PrintStatusLine();
    }

    return true;
}

bool Application::ChangeStatusLine(size_t n, stat_color color)
{
    if (m_sLine.empty())
        return true;

    if (m_sLine[n].color != color)
    {
        //LOG(DEBUG) << " A::ChangeStatusLine n=" << n << " c=" << static_cast<int>(color);
        m_sLine[n].color = color;
        PrintStatusLine();
    }

    return true;
}

bool Application::SwapStatusLine(size_t n)
{
    m_sLine[n].text.swap(m_sLine[n].textAlt);

    PrintStatusLine();
    return true;
}

bool Application::ShowProgressBar(size_t n)
{
    n /= 2;
    if (n == 0 || n > 50)
        return true;

    pos_t y = m_wndManager.m_sizey - 1;

    bool rc = m_wndManager.ColorRect(0, y, static_cast<pos_t>(n), 1, ColorStatusLineB)
    && PrintClock();

    return rc;
}

//////////////////////////////////////////////////////////////////////////////
bool Application::Repaint()
{
    //LOG(DEBUG) << " A::Repaint access menu=" << m_accessMenu.size() << " status line=" << m_sLine.size();

    if (!m_accessMenu.empty())
    {
        pos_t x = 0;
        pos_t y = m_wndManager.m_sizey - m_wndManager.m_bottomLines;

        m_wndManager.StopPaint();
        m_wndManager.GotoXY(x, y);
        m_wndManager.SetTextAttr(ColorAccessMenu);

        for(const auto& m : m_accessMenu)
        {
            size_t size = 0;
            for (size_t i = 0; i < m.name.size() && size < 8 && x < m_wndManager.m_sizex - 1; ++i, ++x, ++size)
            {
                char c = m.name[i];
                if (c != '&')
                    m_wndManager.WriteChar(c);
                else
                {
                    m_wndManager.SetTextAttr(ColorAccessMenuB);
                    m_wndManager.WriteChar(m.name[++i]);
                    m_wndManager.SetTextAttr(ColorAccessMenu);
                }
            }
            for (; size < 8 && x < m_wndManager.m_sizex - 1; ++size, ++x)
                m_wndManager.WriteChar();
        }
        if (x < m_wndManager.m_sizex)
            m_wndManager.FillRect(x, y, m_wndManager.m_sizex - x, 1, ' ', ColorAccessMenu);
        m_wndManager.BeginPaint();
        m_wndManager.ShowBuff(0, y, m_wndManager.m_sizex, 1);
    }

    bool rc = PrintStatusLine()
    && PrintClock(true);

    return rc;
}

bool  Application::PrintClock(bool print)
{
    if (clock_pos::off == m_clock)
        return true;

    time_t curTime = time(nullptr);

    if (curTime <= m_prevClock + 1 && !print)
        return true;
    m_prevClock = curTime;

    tm _tm;
#ifdef WIN32    
    localtime_s(&_tm, &curTime);
#else
    _tm = *std::localtime(&curTime);
#endif

    //LOG(DEBUG) << __FUNC__ << " " << curTime;

    std::stringstream stream;
    if(curTime & 2)
        stream << std::put_time(&_tm, "%H:%M");
    else
        stream << std::put_time(&_tm, "%H %M");

    pos_t len = static_cast<pos_t>(stream.str().size());
    pos_t y = (clock_pos::bottom == m_clock) ? m_wndManager.m_sizey - 1 : 0;

    m_wndManager.StopPaint();
    bool rc = m_wndManager.GotoXY(m_wndManager.m_sizex - len, y)
        && m_wndManager.SetTextAttr(ColorClock)
        && m_wndManager.WriteStr(stream.str());
    
    m_wndManager.BeginPaint();
    rc =  m_wndManager.ShowBuff(m_wndManager.m_sizex - len, y, len, 1);

    return rc;
}

bool  Application::PrintStatusLine()
{
    if (m_sLine.empty())
        return true;

    //LOG(DEBUG) << __FUNC__;
    
    pos_t x = 0;
    pos_t y = m_wndManager.m_sizey - 1;

    m_wndManager.StopPaint();
    bool rc = m_wndManager.GotoXY(x, y);

    auto li = m_sLine.cbegin();
    if (!li->text.empty())
    {
        if (li->color != stat_color::error)
            rc = m_wndManager.SetTextAttr(ColorStatusLine);
        else
            rc = m_wndManager.SetTextAttr(ColorStatusLineB);
        rc = m_wndManager.WriteStr(li->text);
        x += static_cast<pos_t>(li->text.size());
    }

    //fill rest of line
    rc = m_wndManager.SetTextAttr(ColorStatusLine);
    rc = m_wndManager.FillRect(x, y, m_wndManager.m_sizex - x - ((m_clock == clock_pos::bottom) ? 5 : 0), 1, ' ', ColorStatusLine);

    std::stringstream sstr;
    for (++li; li != m_sLine.end() && x < m_wndManager.m_sizex; ++li)
    {
        if (!li->text.empty())
            sstr << "|" << li->text;
    }
    if (m_clock == clock_pos::bottom)
    {
        //place for clock
        sstr << "|";
    }

    if (m_wndManager.m_sizex >= static_cast<pos_t>(sstr.str().size()) + 5)
    {
        rc = m_wndManager.GotoXY(m_wndManager.m_sizex - static_cast<pos_t>(sstr.str().size()) - 5, y); //reserv 5 positions for clock
        li = m_sLine.cbegin();
        for (auto c : sstr.str())
        {
            if (c == '|')
            {
                ++li;
                rc = m_wndManager.SetTextAttr(ColorStatusLineG);
                rc = m_wndManager.WriteChar(c);
                if (li == m_sLine.cend() || li->color == stat_color::normal)
                    rc = m_wndManager.SetTextAttr(ColorStatusLine);
            }
            else
                rc = m_wndManager.WriteChar(c);
        }
    }

    m_wndManager.BeginPaint();
    rc = m_wndManager.ShowBuff(0, y, m_wndManager.m_sizex, 1);

    return rc;
}

//////////////////////////////////////////////////////////////////////////////
input_t Application::MainProc(input_t exit_code)
{
    LOG(DEBUG) << " A::Main " << std::hex << exit_code << std::dec;

    [[maybe_unused]]bool rc = false;
    input_t iKey = 0;

    while ((iKey & K_TYPEMASK) != exit_code && m_inited)
    {
        rc = m_wndManager.CheckRefresh();

        if (m_insert)
            rc = m_wndManager.ShowInputCursor(cursor_t::CURSOR_NORMAL);
        else
            rc = m_wndManager.ShowInputCursor(cursor_t::CURSOR_OVERWRITE);

        rc = m_wndManager.m_console.InputPending();
        if (ConsoleInput::s_fExit)
        {
            SaveCfg(K_CLOSE);
            LOG(DEBUG) << " A::Main fExit";
            return exit_code;
        }

        iKey = m_wndManager.m_console.GetInput();
        if (!m_inited)
            return 0;

        if (!iKey)
        {
            PrintClock();
            if (m_capturedInput)
                iKey = m_capturedInput->EventProc(K_TIME);
            else
                iKey = EventProc(K_TIME);
        }
        else
        {
            //LOG(DEBUG) << __FUNC__ << " key=" << std::hex << iKey << std::dec;
            if (!(iKey & K_RESIZE) && !((iKey & K_TYPEMASK) == K_RELEASE)
                && !m_sLine.empty() && m_sLine[0].color != stat_color::normal)
            {
                //clear error line
                if (iKey == K_ESC && m_sLine[0].color == stat_color::error)
                {
                    ChangeStatusLine(0);
                    continue;
                }
                ChangeStatusLine(0);
            }

            //1 check mouse for access line
            iKey = CheckMouse(iKey);
            if (!iKey)
                continue;

            if (m_capturedInput)
            {
                input_t k = 0;
                if ((iKey & K_TYPEMASK) == K_RESIZE)
                    k = iKey;

                iKey = m_capturedInput->EventProc(iKey);
                if (k)
                {
                    //resize
                    EventProc(k);
                    iKey = 0;
                    break;
                }

                if (!iKey || (iKey & K_TYPEMASK) == K_CLOSE)
                    continue;
            }
            iKey = EventProc(iKey);
        }
    }

    SaveCfg(iKey);
    LOG(DEBUG) << " A::Main exit " << std::hex << iKey << std::dec;
    return iKey;
}

input_t  Application::EventProc(input_t code)
{
    //2 check convert
    auto scan = m_cmdParser.ScanKey(code);
    if(scan == scancmd_t::not_found)
        return ParseCommand(code);
    else if (scan == scancmd_t::collected)
    {
        auto cmdlist = m_cmdParser.GetCommand();
        for (auto cmd : cmdlist)
        {
            code = ParseCommand(cmd);
            if (code == K_EXIT)
                break;
        }
    }

    return code;
}

input_t  Application::CheckMouse(input_t code)
{
    if (0 == (code & K_MOUSE))
        return code;

    if (m_mouseCapture)
    {
        if ((code & K_TYPEMASK) == K_MOUSEKUP)
            m_mouseCapture = false;
        return 0;
    }

    if (m_capturedInput)
        return code;

    uint32_t x = K_GET_X(code);
    pos_t y = K_GET_Y(code);

    if (!y && m_mainMenu)
    {
        //main menu
        if ((code & K_TYPEMASK) == K_MOUSEKL)
        {
            m_wndManager.m_console.PutInput(code);
            code = K_MENU;
        }
    }
    else if (!m_sLine.empty() && y == m_wndManager.m_sizey - 1)
    {
        //status line
        if ((code & K_TYPEMASK) == K_MOUSEKL
         || (code & K_TYPEMASK) == K_MOUSEKM
         || (code & K_TYPEMASK) == K_MOUSEKR)
        {
            code = 0;
            m_mouseCapture = true;
        }
    }
    else if (!m_accessMenu.empty()
         && ((m_sLine.empty() && y == m_wndManager.m_sizey - 1)
            || y == m_wndManager.m_sizey - 2))
    {
        //access menu
        if ((code & K_TYPEMASK) == K_MOUSEKL)
        {
            x /= 8;
            if (x > 9)
                x = 9;
            code = K_F1 + (x << 24);
            m_mouseCapture = true;
        }
        else if ((code & K_TYPEMASK) == K_MOUSEKM
              || (code & K_TYPEMASK) == K_MOUSEKR)
        {
            code = 0;
            m_mouseCapture = true;
        }
    }

    return code;
}

input_t Application::ParseCommand(input_t code)
{
    if((code & K_TYPEMASK) == K_MENU)
    {
        if(m_mainMenu)
            m_mainMenu->Activate(code);
        return 0;
    }

    //4 check app
    code = AppProc(code);

    //5 check user
    if(code)
    {
        LOG_IF(code != K_TIME, INFO) << "  " << ConsoleInput::CastKeyCode(code);

        if(code == K_EXIT)
            SaveCfg(code);

        code = m_wndManager.ProcInput(code);
    }

    return code;
}

//////////////////////////////////////////////////////////////////////////////
bool Application::PutMacro(input_t code)
{
    if (m_recordMacro && code != K_TIME)
        return m_wndManager.m_console.PutMacro(code);
    else
        return true;
}

bool Application::SetCmdParser(const CmdMap& cmdMap)
{
    return m_cmdParser.SetCmdMap(cmdMap);
}

bool Application::CloseWindow([[maybe_unused]] Wnd* wnd)
{
    return true;
}

bool Application::RecordMacro()
{
    if (!m_recordMacro)
    {
        //start record
        m_recordMacro = true;
        m_wndManager.m_console.ClearMacro();
        StatusRecordMacro(true);
    }
    else
    {
        //stop record
        m_recordMacro = false;
        StatusRecordMacro(false);
    }
    return true;
}

bool Application::PlayMacro()
{
    return m_wndManager.m_console.PlayMacro();
}

} //namespace _WndManager 
