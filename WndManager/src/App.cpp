#include "App.h"

#include <iomanip>


//////////////////////////////////////////////////////////////////////////////
bool Application::Init()
{
    LOG(DEBUG) << " A::Init";
    if (m_inited)
        true;

    bool rc = m_wndManager.Init();
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

//////////////////////////////////////////////////////////////////////////////
bool Application::Repaint()
{
    LOG(DEBUG) << " A::Repaint a=" << m_accessMenu.size() << " s=" << m_sLine.size();

    if (!m_accessMenu.empty())
    {
        pos_t x = 0;
        pos_t y = m_wndManager.m_sizey - m_wndManager.m_bottomLines;

        m_wndManager.StopPaint();
        m_wndManager.GotoXY(x, y);
        m_wndManager.SetTextAttr(ColorAccessMenu);

        for(const auto& m : m_accessMenu)
        {
            const std::string name = m.name;
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

    LOG(DEBUG) << __FUNCTION__;

    time_t curTime = time(nullptr);

    if (curTime <= m_prevTime + 1 && !print)
        return true;

    m_prevTime = curTime;

    tm _tm;
    localtime_s(&_tm, &curTime);
    std::stringstream stream;
    if(curTime & 1)
        stream << std::put_time(&_tm, "H:M");
    else
        stream << std::put_time(&_tm, "H M");

    pos_t len = (pos_t)stream.str().size();
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

    LOG(DEBUG) << __FUNCTION__;
    
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
        x += (pos_t)li->text.size();
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

    rc = m_wndManager.GotoXY(m_wndManager.m_sizex - (pos_t)sstr.str().size() - 5, y); //reserv 5 positions for clock
    li = m_sLine.cbegin();
    for (auto c : sstr.str())
    {
        if (c == '|')
        {
            ++li;
            rc = m_wndManager.SetTextAttr(ColorStatusLineG);
            rc = m_wndManager.WriteChar(c);
            if (li->color == stat_color::normal)
                rc = m_wndManager.SetTextAttr(ColorStatusLine);
        }
        else
            rc = m_wndManager.WriteChar(c);
    }

    m_wndManager.BeginPaint();
    rc = m_wndManager.ShowBuff(0, y, m_wndManager.m_sizex, 1);

    return rc;
}

//////////////////////////////////////////////////////////////////////////////
input_t Application::MainProc(input_t exit_code)
{
    LOG(DEBUG) << " A::Main " << std::hex << exit_code << std::dec;

    bool rc = false;
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
    //if (!m_pKeyConv) //???
        return ParseCommand(code);
/*
    int rc = m_pKeyConv->ScanKey(code);
    if (rc < 0)
        code = ParseCommand(code);
    else if (rc > 0)
        while ((code = m_pKeyConv->GetCommand()) != 0)
        {
            code = ParseCommand(code);
            if (code == K_EXIT)
                break;
        }

    return code;
*/
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
        if(code != K_TIME)
            LOG(INFO) << "  " << ConsoleInput::CastKeyCode(code);

        if(code == K_EXIT)
            SaveCfg();

        [[maybe_unused]]bool rc = m_wndManager.ProcInput(code);
    }

    return code;
}

//////////////////////////////////////////////////////////////////////////////
bool Application::ChangeStatusLine(size_t n, std::optional<std::string_view> text, stat_color color)
{
    if (m_sLine.empty())
        return true;

    LOG(DEBUG) << " A::ChangeStatusLine n=" << n << " '" << text.value_or("") << "' c=" << static_cast<int>(color);

    if (n >= m_sLine.size())
    {
        LOG(ERROR) << __FUNCTION__ << " error n=" << n << " > size=" << m_sLine.size();
        return false;
    }

    if (n == 0 && m_sLine[0].color == stat_color::error && text.has_value() && color != stat_color::error)
        //save error line until input event
        return true;

    if (m_sLine[n].text.empty() && !text.has_value())
        return true;

    if (m_sLine[n].text.empty() || !text.has_value() || m_sLine[n].text != text || m_sLine[n].color != color)
    {
        m_sLine[n].text = text.value_or("");
        m_sLine[n].color = color;
        PrintStatusLine();
    }

    return 0;
}


#if 0

int  Application::SetLogo(Logo* pLogo)
{
  return m_wndManager.SetLogo(pLogo);
}


int  Application::SetAccessMenu(menu* pMenu)
{
  int rc = 0;
  m_pAccessMenu = pMenu;
  if(pMenu)
  {
    if(m_pSLine)
      m_wndManager.m_BottomLine = 2;
    else
      m_wndManager.m_BottomLine = 1;
  }
  else
  {
    if(m_pSLine)
      m_wndManager.m_BottomLine = 1;
    else
      m_wndManager.m_BottomLine = 0;
  }
  m_wndManager.CalcView();

  return rc;
}


int  Application::SetStatusLine(statline* pLine)
{
  int rc = 0;
  m_pSLine = pLine;
  if(pLine)
  {
    if(m_pAccessMenu)
      m_wndManager.m_BottomLine = 2;
    else
      m_wndManager.m_BottomLine = 1;
  }
  else
  {
    if(m_pAccessMenu)
      m_wndManager.m_BottomLine = 1;
    else
      m_wndManager.m_BottomLine = 0;
  }
  m_wndManager.CalcView();

  return rc;
}


int  Application::Refresh()
{
  return m_wndManager.Refresh();
}


int  Application::Repaint()
{
  TPRINT((" A::Repaint b=%d s=%d\n",
    m_pAccessMenu != NULL, m_pSLine != NULL));

  int rc = 0;

  if(m_pAccessMenu)
  {
    short x = 0;
    short y = m_wndManager.m_sizey - m_wndManager.m_BottomLine;

    m_wndManager.StopPaint();
    m_wndManager.GotoXY(x, y);
    m_wndManager.SetTextAttr(ColorAccessMenu);
    for(menu* m = m_pAccessMenu; m->pName && x < m_wndManager.m_sizex - 1; ++m)
    {
      const char* pName = GetSStr(m->pName);
      int i;
      int n = 0;
      for(i = 0; pName[i] && n < 8; ++i, ++x)
      {
        char c = pName[i];
        if(c != '&')
          m_wndManager.WriteChar(c);
        else
        {
          m_wndManager.SetTextAttr(ColorAccessMenuB);
          m_wndManager.WriteChar(pName[++i]);
          m_wndManager.SetTextAttr(ColorAccessMenu);
        }
        ++n;
      }
      for(;n < 8; ++n, ++x)
        m_wndManager.WriteChar();
    }
    if(x < m_wndManager.m_sizex)
      m_wndManager.FillRect(x, y, m_wndManager.m_sizex - x, 1, ' ', ColorAccessMenu);
    m_wndManager.BeginPaint();
    m_wndManager.ShowBuff(0, y, m_wndManager.m_sizex, 1);
  }

  rc = PrintStatusLine();
  rc = PrintClock(1);

  return rc;
}


int  Application::PrintClock(int fPrint)
{
  if(!m_fClock)
    return 0;

  //TPRINT((" A::PrintClock\n"));

  static int n = 0;
  static time_t prevTime = 0;
  time_t curTime = time(NULL);

  if(curTime <= prevTime + 1 && !fPrint)
    return 0;

  if(curTime > prevTime + 1)
    n = (n) ? 0 : 1;

  prevTime = curTime;
  tm Tm;
  int rc = localtime_s(&Tm, &curTime);

  char buff[32];
  snprintf(buff, sizeof(buff) - 1, "%02d%c%02d", Tm.tm_hour, (n) ? ':' : ' ', Tm.tm_min);

  short y = (m_fClock == 2) ? m_wndManager.m_sizey - 1 : 0;
  m_wndManager.StopPaint();
  rc = m_wndManager.GotoXY(m_wndManager.m_sizex - (short)strlen(buff), y);
  rc = m_wndManager.SetTextAttr(ColorClock);
  rc = m_wndManager.WriteStr(buff);
  m_wndManager.BeginPaint();
  rc = m_wndManager.ShowBuff(m_wndManager.m_sizex - (short)strlen(buff), y, (short)strlen(buff), 1);

  return rc;
}


int  Application::ShowProgressBar(short n)
{
  n /= 2;
  if(n <= 0 || n > 50)
    return 0;

  short y = m_wndManager.m_sizey - 1;

  int rc = m_wndManager.ColorRect(0, y, n, 1, ColorStatusLineB);
  rc = PrintClock();

  return rc;
}


int  Application::PrintStatusLine()
{
  if(!m_pSLine)
    return 0;

  //TPRINT((" A::PrintStatusLine\n"));
  short x = 0;
  short y = m_wndManager.m_sizey - 1;

  m_wndManager.StopPaint();
  int rc = m_wndManager.GotoXY(x, y);

  statline* s = m_pSLine;
  if(s->pText)
  {
    if(s->color != 2)
      rc = m_wndManager.SetTextAttr(ColorStatusLine);
    else
      rc = m_wndManager.SetTextAttr(ColorStatusLineB);
    rc = m_wndManager.WriteStr(GetSStr(s->pText));
    x += (short)strlen(GetSStr(s->pText));
  }

  //fill rest of line
  rc = m_wndManager.SetTextAttr(ColorStatusLine);
  rc = m_wndManager.FillRect(x, y, m_wndManager.m_sizex - x - ((m_fClock == 2) ? 5 : 0), 1, ' ', ColorStatusLine);

  char buff[256] = {0};
  short l = 0;
  for(++s; s->pText && x < m_wndManager.m_sizex; ++s)
  {
    if(s->pText)
      l += (short)snprintf(buff + l, sizeof(buff) - 1 - l, "|%s", GetSStr(s->pText));
  }
  if(m_fClock == 2)
  {
    strcat_s(buff, "|");
    //reserv place for clock
    l = (short)strlen(buff) + 5;
  }

  rc = m_wndManager.GotoXY(m_wndManager.m_sizex - l, y);
  s = m_pSLine;
  for(int i = 0; buff[i]; ++i)
  {
    if(buff[i] == '|')
    {
      ++s;
      rc = m_wndManager.SetTextAttr(ColorStatusLineG);
      rc = m_wndManager.WriteChar(buff[i]);
      if(!s->color)
        rc = m_wndManager.SetTextAttr(ColorStatusLine);
    }
    else
      rc = m_wndManager.WriteChar(buff[i]);
  }
  m_wndManager.BeginPaint();
  rc = m_wndManager.ShowBuff(0, y, m_wndManager.m_sizex, 1);

  return rc;
}


int  Application::ChangeStatusLine(int n, const char* pText, color_t color)
{
  if(!m_pSLine)
    return 0;

  //TPRINT((" A::ChangeStatusLine n=%d %s gr=%d\n", n, GetSStr(pText), color));

  if(n == 0 && m_pSLine[n].color == 2 && pText && color != 2)
    //save error line until input event
    return 0;

  if(!m_pSLine[n].pText && !pText)
    return 0;

  if(!m_pSLine[n].pText || !pText
  || strcmp(GetSStr(m_pSLine[n].pText), GetSStr(pText)) || m_pSLine[n].color != color)
  {
    if(pText)
    {
      strcpy_s(m_sLine, GetSStr(pText));
      m_pSLine[n].pText = m_sLine;
    }
    else
      m_pSLine[n].pText = pText;
    m_pSLine[n].color = color;
    PrintStatusLine();
  }
//  else
//    TPRINT(("Not Changed\n"));

  return 0;
}


int  Application::ChangeStatusLine(int n, color_t color)
{
  if(!m_pSLine)
    return 0;

  if(m_pSLine[n].color != color)
  {
    //TPRINT((" A::ChangeStatusLine n=%d gr=%d\n", n, color));
    m_pSLine[n].color = color;
    PrintStatusLine();
  }

  return 0;
}


int  Application::SwapStatusLine(int n)
{
  const char* pText         = m_pSLine[n].pText;
  m_pSLine[n].pText    = m_pSLine[n].pTextAlt;
  m_pSLine[n].pTextAlt = pText;

  PrintStatusLine();
  return 0;
}


int  Application::PutCode(int cmd)
{
  return m_Console->PutInput(cmd);
}


int Application::RecordMacro()
{
  if(!m_fRecordMacro)
  {
    //start record
    m_fRecordMacro = 1;
    m_Console->ClearMacro();
    StatusRecordMacro(1);
  }
  else
  {
    //stop record
    m_fRecordMacro = 0;
    StatusRecordMacro(0);
  }
  return 0;
}


int Application::PutMacro(int cmd)
{
  if(m_fRecordMacro && cmd != K_TIME)
    return m_Console->PutMacro(cmd);
  else
    return 0;
}


int Application::PlayMacro()
{
  return m_Console->PlayMacro();
}

int Application::SetKeyConv(int* pConv)
{
    if (m_pKeyConv)
        delete m_pKeyConv;
    m_pKeyConv = new KeyConv(pConv);
    return 0;
}



#endif

std::string Application::GetKeyName(input_t code)
{
    return "F?";
}
