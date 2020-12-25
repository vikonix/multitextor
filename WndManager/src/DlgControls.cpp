#include "DlgControls.h"
#include "ConsoleScreen.h"
#include "utils/logger.h"


/////////////////////////////////////////////////////////////////////////////
bool Control::SetName(const std::string& name)
{
    m_name = name;
    m_sizex = (pos_t)m_name.size() + m_addSize;

    return true;
}

bool Control::Paint(const std::string& str, int type)
{
    pos_t x = m_posx;
    pos_t y = m_posy;

    color_t color;
    if(type & CTRL_DISABLED)
        color = ColorDialogDisabled;
    else if(type & CTRL_SELECTED)
        color = ColorDialogSelect;
    else
        color = *m_dialog.m_pColorWindow;

    std::u16string wstr;
    for(size_t i = 0; i < str.size(); ++i)
    {
        char c = str[i];
        if(c == '&')
        {
            c = str[++i];
            m_key = std::toupper(c);
            if(!(type & CTRL_DISABLED))
                m_dialog.WriteChar(x, y, c, ColorDialogInfo);
            else
                m_dialog.WriteChar(x, y, c, color);
        }
        else
            m_dialog.WriteChar(x, y, c, color);

        if(c == ']' || c == ')')
        {
            if(type & CTRL_SELECTED)
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
    if(m_sizex == 0 && m_name.size() != 0)
        m_sizex = (pos_t)m_name.size();
    if(m_sizex < (pos_t)m_name.size())
        m_name.resize(m_sizex);
}

bool CtrlStatic::Refresh([[maybe_unused]]CtrlState state)
{
    LOG(DEBUG) << "     CtrlStatic::Refresh id=" << m_pos << " name=" << m_name;
    color_t color;

    if((m_type & CTRL_TYPE_MASK) == CTRL_TITLE)
    {
        color = *m_dialog.m_pColorWindowTitle;
        std::string str{ ' ' + m_name + ' ' };
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

        std::string str((size_t)m_sizex + amp, acs_t::ACS_HLINE);
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
            m_dialog.WriteStr(m_posx, m_posy, m_name, color);
        }
        else
        {
            color = ColorDialogDisabled;
            m_dialog.WriteStr(m_posx, m_posy, m_name, color);
        }
    }

    return true;
}

bool CtrlStatic::SetName(const std::string& name)
{
    pos_t oldLen = (pos_t)m_name.size();

    //we save max string size
    pos_t sizex = m_sizex;
    Control::SetName(name);
    m_sizex = sizex;

    if((m_type & CTRL_TYPE_MASK) != CTRL_TITLE)
    {
        if(m_sizex < m_name.size())
            m_name.resize(m_sizex);

        Refresh();
        pos_t newLen = (pos_t)m_name.size();
        if(newLen < oldLen)
        {
            color_t color = *m_dialog.m_pColorWindow;
            m_dialog.FillRect(m_posx + newLen, m_posy, oldLen - newLen, 1, ' ', color);
        }
    }
    else
    {
        //???
        _assert(0);
    }

    return true;
}

#if 0
/////////////////////////////////////////////////////////////////////////////
CtrlButton::CtrlButton(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, (short)strlen(GetSStr(pControl->pName)), 1, pControl->pHelpLine)
{
  if(strchr(m_pName, '&'))//&
    --m_sizex;

  if((m_nType & CTRL_TYPE_MASK) == CTRL_BUTTON)
  {
    m_dcursorx = 2;
    m_sizex += m_nAddSize = 4;

  }
  else
  {
    m_dcursorx = 3;
    m_sizex += m_nAddSize = 6;
  }
}


int CtrlButton::Refresh(int type)
{
  //TPRINT(("     CtrlButton::Refresh id=%d name=%s\n", m_nId, m_pName));
  char buff[128];
  if((m_nType & CTRL_TYPE_MASK) == CTRL_BUTTON)
    sprintf_s(buff, sizeof(buff), "[ %s ]", m_pName);
  else
    sprintf_s(buff, sizeof(buff), "[_ %s _]", m_pName);

  Paint(buff, type | (m_nType & CTRL_STATE_MASK));

  return 0;
}


int CtrlButton::EventProc(int code)
{
  if(code == K_SPACE
  || code == K_ENTER)
  {
    return m_nId;
  }
  else if(code & K_MOUSE)
  {
    if((code & K_TYPEMASK) == K_MOUSEKUP)
      return m_nId;
  }

  return code;
}


/////////////////////////////////////////////////////////////////////////////
CtrlCheck::CtrlCheck(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, (short)strlen(GetSStr(pControl->pName)), 1, pControl->pHelpLine)
{
  m_dcursorx = 1;
  m_sizex += m_nAddSize = 4;

  if(m_pVar)
    m_nChecked = *((int*)m_pVar);
  else
    m_nChecked = 0;
}


int CtrlCheck::Refresh(int type)
{
  //TPRINT(("     CtrlCheck::Refresh id=%d name=%s\n", m_nId, m_pName));
  char buff[128];
  if(!m_nChecked)
    sprintf_s(buff, sizeof(buff), "[ ] %s", m_pName);
  else
    sprintf_s(buff, sizeof(buff), "[x] %s", m_pName);

  Paint(buff, type | (m_nType & CTRL_STATE_MASK));

  return 0;
}


int CtrlCheck::EventProc(int code)
{
  if(code == K_SPACE)
  {
    m_nChecked = m_nChecked ? 0 : 1;
    Refresh(CTRL_SELECTED);
    return 0;
  }
  else if(code & K_MOUSE)
  {
    if((code & K_TYPEMASK) == K_MOUSEKUP)
    {
      m_nChecked = m_nChecked ? 0 : 1;
      Refresh(CTRL_SELECTED);
      return 0;
    }
  }

  return code;
}


int CtrlCheck::UpdateVar()
{
  if(m_pVar)
    *((int*)m_pVar) = m_nChecked;
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
CtrlRadio::CtrlRadio(Dialog* pDialog, control* pControl, int index)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, (short)strlen(GetSStr(pControl->pName)), 1, pControl->pHelpLine)
{
  m_dcursorx = 1;
  m_sizex += m_nAddSize = 4;

  m_nIndex = index;

  if(m_pVar)
  {
    if(*((int*)m_pVar) == m_nIndex)
      m_nChecked = 1;
    else
      m_nChecked = 0;
  }
  else if(!m_nIndex)
    m_nChecked = 1;
  else
    m_nChecked = 0;

  //TPRINT(("CtrlRadio id=%d var=%d ch=%d\n", m_nIndex, *((int*)m_pVar), m_nChecked));
}


int CtrlRadio::Refresh(int type)
{
  //TPRINT(("     CtrlRadio::Refresh id=%d name=%s\n", m_nId, m_pName));
  char buff[128];
  if(!m_nChecked)
    sprintf_s(buff, sizeof(buff), "( ) %s", m_pName);
  else
    sprintf_s(buff, sizeof(buff), "(*) %s", m_pName);

  Paint(buff, type | (m_nType & CTRL_STATE_MASK));

  return 0;
}


int CtrlRadio::EventProc(int code)
{
  int fSelect = 0;

  if(code == K_SPACE)
    fSelect = 1;
  else if(code & K_MOUSE)
    if((code & K_TYPEMASK) == K_MOUSEKUP)
      fSelect = 1;

  if(fSelect)
  {
    SetCheck();
    return 0;
  }
  else
    return code;
}


int CtrlRadio::UpdateVar()
{
  if(m_pVar && m_nChecked)
    *((int*)m_pVar) = m_nIndex;
  return 0;
}


int CtrlRadio::SetCheck()
{
  int c = m_nChecked;
  int f = m_dialog.CtrlRadioChecked(this);
  if(f)
    m_nChecked = 1;
  else
  {
    //alone radio button
    m_nChecked = c ? 0 : 1;
  }

  Refresh(CTRL_SELECTED);
  return m_nChecked;
}


/////////////////////////////////////////////////////////////////////////////
CtrlGroup::CtrlGroup(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, pControl->sizex, pControl->sizey)
{
}


int CtrlGroup::Refresh(int /*type*/)
{
  //TPRINT(("     CtrlGroup::Refresh id=%d name=%s\n", m_nId, m_pName));
  color_t color = *m_dialog.m_pColorWindow;

  size_t size = m_sizex + (strchr(m_pName, '&') ? 1 : 0);
  char* pBuff = new char[size + 1];

  pBuff[0] = ACS_ULCORNER;
  strcpy_s(pBuff + 1, size, m_pName);

  size_t nl = strlen(pBuff);
  memset(pBuff + nl, ACS_HLINE, size - 1 - nl);
  pBuff[size - 1] = ACS_URCORNER;
  pBuff[size]     = 0;
  Paint(pBuff, 0);

  m_dialog.FillRect(m_posx,               m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);
  m_dialog.FillRect(m_posx - 1 + m_sizex, m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);

  pBuff[0] = ACS_LLCORNER;
  memset(pBuff + 1, ACS_HLINE, m_sizex - 2);
  pBuff[m_sizex - 1] = ACS_LRCORNER;
  pBuff[m_sizex]     = 0;
  m_dialog.WriteStr(m_posx, m_posy + m_sizey - 1, pBuff, color);

  delete pBuff;

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
CtrlEdit::CtrlEdit(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, pControl->sizex, 1, pControl->pHelpLine)
{
  m_pBuff = new char [pControl->sizex + 1];
  if(!m_pBuff)
    return;

  m_nBuffLen = pControl->sizex;
  memset(m_pBuff, ' ', m_nBuffLen);
  m_pBuff[m_nBuffLen] = 0;

  m_nLen     = 0;
  m_nPos     = 0;

  if(m_pVar)
  {
    strncpy_s(m_pBuff, m_nBuffLen + 1, (char*)m_pVar, m_nBuffLen);

    m_nLen = (short)strlen(m_pBuff);
    if(m_nLen < m_nBuffLen)
    {
      //fill rest of strinf with spaces
      memset(m_pBuff + m_nLen, ' ', m_nBuffLen - m_nLen);
      m_pBuff[m_nBuffLen] = 0;
    }
  }

  m_nBeginSel = 0;
  m_nEndSel   = 0;
}


int CtrlEdit::Refresh(int type)
{
  //TPRINT(("     CtrlEdit::Refresh id=%d buff=<%s>\n", m_nId, m_pBuff));

  color_t color;
  if(m_nType & CTRL_DISABLED)
    color = ColorDialogDisabled;
  else if(type & CTRL_SELECTED)
  {
    if(!m_nEndSel)
      color = ColorDialogSelect;
    else
      color = ColorDialogFieldSel;
  }
  else
    color = ColorDialogField;

  m_dialog.WriteStr(m_posx, m_posy, m_pBuff, color);

  return 0;
}


short CtrlEdit::EndSelect(int t)
{
  //0-unselect 1-del
  if(t == 0)
  {
    if(m_nEndSel - m_nBeginSel)
    {
      m_nBeginSel = 0;
      m_nEndSel   = 0;
      Refresh(CTRL_SELECTED);
    }
  }
  else if(t == 1)
  {
    short n = m_nEndSel - m_nBeginSel;
    if(n)
    {
      memset(m_pBuff, ' ', m_nBuffLen);
      m_nLen     -= n;
      m_nBeginSel = 0;
      m_nEndSel   = 0;

      m_nPos      = 0;
      m_dcursorx  = m_nPos;
      m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);

      Refresh(CTRL_SELECTED);
    }
  }
  return m_nPos;
}


int CtrlEdit::EventProc(int code)
{
  short x = m_nPos;
  int f = 0;

  if(code == K_LEFT)
  {
    EndSelect();
    if(x)
      --x;
  }
  else if(code == K_RIGHT)
  {
    if(m_nEndSel - m_nBeginSel)
      EndSelect();
    else
      if(x < m_nBuffLen - 1)
        ++x;
  }
  else if(code == K_HOME)
  {
    EndSelect();
    x = 0;
  }
  else if(code == K_END)
  {
    EndSelect();
    if(m_nLen < m_nBuffLen)
      x = m_nLen;
    else
      x = m_nLen - 1;
  }
  else if(code == K_BS)
  {
    x = EndSelect(1);
    if(x)
    {
      if(x > m_nLen)
        --x;
      else
      {
        --x;
        //del char at x
        memmove(m_pBuff + x, m_pBuff + x + 1, m_nBuffLen - x);
        m_pBuff[m_nBuffLen - 1] = ' ';
        --m_nLen;
        f = 1;
      }
    }
  }
  else if(code == K_DELETE)
  {
    x = EndSelect(1);
    if(m_nLen && x <= m_nLen)
    {
      //del char at x
      memmove(m_pBuff + x, m_pBuff + x + 1, m_nBuffLen - x);
      m_pBuff[m_nBuffLen - 1] = ' ';
      --m_nLen;
      f = 1;
    }
  }
  else if(code == (K_INSERT | K_SHIFT)
       || code == ('V' | K_CTRL))
  {
    //TPRINT(("     Paste\n"));
    x = EndSelect(1);

    int l = m_nBuffLen - x;
    char* pBuff  = new char[l];
    if(!pBuff)
      return 0;

    if(!PasteFromClipboard(pBuff, l))
    {
      delete pBuff;
      return 0;
    }

    char* pBuff1 = new char[l];
    if(!pBuff1)
    {
      delete pBuff;
      return 0;
    }

    short j = 0;
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

    memmove(m_pBuff + x + j, m_pBuff + x, m_nBuffLen - x - j);
    memcpy(m_pBuff + x, pBuff1, j);
    m_nLen = x + j;
    f = 1;

    delete pBuff;
    delete pBuff1;
  }
  else if((code & K_TYPEMASK) == K_SYMBOL)
  {
    if(((code & K_MODMASK) & ~K_SHIFT) == 0
    && (code & K_CODEMASK) >= K_SPACE)
    {
      x = EndSelect(1);
      code &= ~K_SHIFT;
      char c = wchar2char(g_textCP, (wchar)code);
      //edit symbol
      if(g_pApplication->GetInsertMode())
      {
        //insert symbol
        memmove(m_pBuff + x + 1, m_pBuff + x, m_nBuffLen - x - 1);
      }
      m_pBuff[x] = c;

      if(m_nLen < x)
        m_nLen = x + 1;
      else if(m_nLen < m_nBuffLen)
        ++m_nLen;
      f = 1;

      if(x < m_nBuffLen - 1)
        ++x;
    }
    else
      return code;
  }
  else if(code & K_MOUSE)
  {
    if((code & K_TYPEMASK) == K_MOUSEKL)
    {
      EndSelect();
      x = K_GET_X(code) - m_posx;
    }
  }
  else
    return code;

  if(x != m_nPos)
  {
    m_dcursorx = m_nPos = x;
    m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
  }
  if(f)
    Refresh(CTRL_SELECTED);

  //TPRINT(("x=%d len=%d s=%d\n", m_nPos, m_nLen, m_nBuffLen));
  return 0;
}


int CtrlEdit::UpdateVar()
{
  if(m_pVar)
  {
    char c = m_pBuff[m_nLen];
    m_pBuff[m_nLen] = 0;
    strcpy_s((char *)m_pVar, m_nLen + 1, m_pBuff);
    m_pBuff[m_nLen] = c;
  }

  return 0;
}


size_t CtrlEdit::GetName(char* pBuff, size_t len)
{
  if(pBuff && len > m_nLen)
  {
    char c = m_pBuff[m_nLen];
    m_pBuff[m_nLen] = 0;
    strcpy_s((char *)pBuff, len, m_pBuff);
    m_pBuff[m_nLen] = c;
  }

  return m_nLen;
}


int CtrlEdit::SetName(const char* pName)
{
  memset(m_pBuff, ' ', m_nBuffLen);
  m_pBuff[m_nBuffLen] = 0;

  m_dcursorx = 0;
  m_nLen     = 0;
  m_nPos     = 0;

  strncpy_s(m_pBuff, m_nBuffLen + 1, GetSStr(pName), m_nBuffLen);

  m_nPos = m_nLen = (short)strlen(m_pBuff);
  if(m_nLen < m_nBuffLen)
  {
    //fill rest of string with spaces
    memset(m_pBuff + m_nLen, ' ', m_nBuffLen - m_nLen);
    m_pBuff[m_nBuffLen] = 0;
  }

  Refresh();

  return 0;
}


int CtrlEdit::Select()
{
  m_nBeginSel = 0;
  m_nEndSel   = m_nLen;

  if(m_nLen < m_nBuffLen)
    m_nPos = m_nLen;
  else
    m_nPos = m_nLen - 1;

  m_dcursorx = m_nPos;
  m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
CtrlList::CtrlList(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, pControl->sizex, pControl->sizey, pControl->pHelpLine)
{
  m_nSelected  = 0;
  m_nFirstLine = 0;
  m_nMouse2    = 0;

  m_dcursorx   = 1;
  m_dcursory   = 1;

  m_pList = new List;
}


CtrlList::~CtrlList()
{
  if(m_pList)
    delete m_pList;
}


int CtrlList::Refresh(int type)
{
  //TPRINT(("     CtrlList::Refresh id=%d name=%s\n", m_nId, m_pName));
  m_dialog.StopPaint();
  char* pBuff = new char[m_sizex + 2];

  color_t color;
  if(m_nType & CTRL_DISABLED)
    color = ColorDialogDisabled;
  else
    color = *m_dialog.m_pColorWindow;

  pBuff[0] = ACS_ULCORNER;
  strcpy_s(pBuff + 1, m_sizex + 1, m_pName);
  size_t nl = strlen(pBuff);
  int amp = 0;
  if(strchr(m_pName, '&'))//&
    amp = 1;
  memset(pBuff + nl, ACS_HLINE, m_sizex + amp - 1 - nl);
  pBuff[m_sizex + amp - 1] = ACS_URCORNER;
  pBuff[m_sizex + amp] = 0;
  Paint(pBuff, m_nType);

  int pos = -1;
  if(m_sizey - 2 > 3)
  {
    size_t size = GetStrCount();
    if(size >= m_sizey - 2)
      pos = (int)(GetSelect() * (m_sizey - 2) / size);
  }

  for(short y = 0; y < m_sizey - 2; ++y)
  {
    m_dialog.WriteChar(m_posx, m_posy + 1 + y, ACS_VLINE, color);

    size_t l;
    char* pLine = m_pList->GetStr(m_nFirstLine + y, &l);
    if(pLine)
    {
      if(l < 0)
        l = 0;
      int fCut = 0;
      if(l > m_sizex - 2)
      {
        l = m_sizex - 2;
        if(pLine[l])
          //if last symbol 0
          fCut = 1;
      }
      if(l)
        memcpy(pBuff, pLine, l);
      if(fCut)
        pBuff[l - 1] = '~';//'>';
      pBuff[l] = 0;
    }
    else
    {
      pBuff[0] = 0;
    }

    l = m_sizex - 1 - strlen(pBuff);
    if(l > 0)
      memset(pBuff + strlen(pBuff), ' ', l);
    pBuff[m_sizex - 2] = 0;

    if(m_nType & CTRL_DISABLED)
      m_dialog.WriteStr(m_posx + 1, m_posy + 1 + y, pBuff, ColorDialogDisabled);
    else if(y == m_nSelected)
    {
      if(type & CTRL_SELECTED)
        m_dialog.WriteStr(m_posx + 1, m_posy + 1 + y, pBuff, ColorDialogSelect);
      else
        m_dialog.WriteStr(m_posx + 1, m_posy + 1 + y, pBuff, ColorDialogFieldAct);
    }
    else
      m_dialog.WriteStr(m_posx + 1, m_posy + 1 + y, pBuff, ColorDialogField);

    if(y != pos)
      m_dialog.WriteChar(m_posx + m_sizex - 1, m_posy + 1 + y, ACS_VLINE, color);
    else
      //current pos in list
      m_dialog.WriteChar(m_posx + m_sizex - 1, m_posy + 1 + y, '+', color);
  }

  pBuff[0] = ACS_LLCORNER;
  memset(pBuff + 1, ACS_HLINE, m_sizex - 2);
  pBuff[m_sizex - 1] = ACS_LRCORNER;
  pBuff[m_sizex] = 0;
  m_dialog.WriteStr(m_posx, m_posy + m_sizey - 1, pBuff, color);

  delete pBuff;

  m_dialog.BeginPaint();
  m_dialog.ShowBuff(m_posx, m_posy, m_sizex, m_sizey);

  return 0;
}


int CtrlList::UpdateVar()
{
  if(m_pVar)
    *((int*)m_pVar) = (int)(m_nFirstLine + m_nSelected);

  return 0;
}


int CtrlList::EventProc(int code)
{
  int n = (int)(m_nFirstLine + m_nSelected);//can be negative
  size_t size = m_pList->GetStrCount();

  if(code == K_TIME)
  {
    //TPRINT(("K_TIME %x\n", m_nMouseCmd));
    code = m_nMouseCmd;
  }

  if((code & K_TYPEMASK) == K_SYMBOL && code != K_BS)
  {
    if((code & K_MODMASK) == 0 && (code & K_CODEMASK) > K_SPACE)
    {
      int f = -1;
      char buff[2] = {wchar2char(g_textCP, wchar(code & K_CODEMASK)), 0};
      f = m_pList->FindSorted(buff, g_textCP, 0);
      if(f != -1)
        n = f;
      //else
      //  n = size - 1;
    }
    else
      return code;
  }
  else if(code == K_UP)
    --n;
  else if(code == K_DOWN)
    ++n;
  else if(code == K_PAGEUP)
  {
    int d = m_sizey - 3;
    n -= d ? d : 1;
  }
  else if(code == K_PAGEDN)
  {
    int d = m_sizey - 3;
    n += d ? d : 1;
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
      if(m_nMouse2)
      {
        m_nMouse2 = 0;
        return K_ENTER;
      }
    }

    if((code & K_TYPEMASK) == K_MOUSEKL)
    {
      int p = K_GET_Y(code) - m_posy;
      if(p <= 0)
        m_nMouseCmd = K_MOUSEWUP;
      else if(p >= m_sizey - 1)
        m_nMouseCmd = K_MOUSEWDN;
      else
        m_nMouseCmd = 0;

      n = m_nFirstLine + p - 1;
      if(code & K_MOUSE2 && !m_nMouseCmd)
      {
        m_nMouse2 = 1;
        return 0;
      }
    }
    else if((code & K_TYPEMASK) == K_MOUSEWUP)
    {
      int d = (m_sizey - 2) / 3;
      n -= d ? d : 1;
    }
    else if((code & K_TYPEMASK) == K_MOUSEWDN)
    {
      int d = (m_sizey - 2) / 3;
      n += d ? d : 1;
    }
  }
  else if(code == K_SPACE)
    return 0;
  else
    return code;

  if(n < 0)
    n = 0;
  if(n >= size)
    n = (int)(size - 1);

  if(n != m_nFirstLine + m_nSelected)
  {
    if(n - m_nFirstLine > m_sizey - 3)
      m_nFirstLine = n - (m_sizey - 3);
    else if(n - m_nFirstLine < 0)
      m_nFirstLine = n;
    m_nSelected = n - m_nFirstLine;
    //TPRINT(("n=%d first=%d sel=%d\n", n, m_nFirstLine, m_nSelected));

    m_dcursory = (short)(m_nSelected + 1);
    m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
    Refresh(CTRL_SELECTED);
    return K_SELECT + n;
  }

  return 0;
}


int CtrlList::SetSelect(int n, int fRefresh)
{
  int count = (int)(m_pList->GetStrCount() - 1);
  if(n > count)
    n = count;

  if(n != m_nFirstLine + m_nSelected)
  {
    if(n == -1)
      n = count;

    if(n - m_nFirstLine > m_sizey - 2 - 3)
    {
      m_nFirstLine = n - (m_sizey - 2 - 3);

      if(m_nFirstLine + m_sizey - 3 > count)
        m_nFirstLine = count - (m_sizey - 3);

      if(m_nFirstLine < 0)
        m_nFirstLine = 0;
    }
    else if(n - m_nFirstLine < 0)
      m_nFirstLine = n;
    m_nSelected = n - m_nFirstLine;
    //TPRINT(("n=%d first=%d sel=%d count=%d\n", n, m_nFirstLine, m_nSelected, count));

    m_dcursory = (short)(m_nSelected + 1);
    if(fRefresh)
    {
      m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
      Refresh();
    }
  }

  return n;
}


/////////////////////////////////////////////////////////////////////////////
CtrlSList::CtrlSList(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, pControl->sizex, pControl->sizey, pControl->pHelpLine),
 m_List(pDialog, pControl)
{
  m_fListOpen = 0;

  m_List.m_posy  += 1;
  m_List.m_sizey -= 1;
}


int CtrlSList::Select()
{
  //TPRINT(("CtrlSList::Select id=%x\n", m_nId));

  m_dcursorx = m_sizex - 2;
  m_dcursory = 0;

  return 0;
}


int CtrlSList::EventProc(int code)
{
  //TPRINT(("CtrlSList::EventProc id=%x code=%x\n", m_nId, code));

  if((code & K_TYPEMASK) == K_MOUSEKUP)
  {
    int x = K_GET_X(code);
    int y = K_GET_Y(code);

    //TPRINT(("Mouse x=%d y=%d\n", x, y));

    if(!m_fListOpen)
      if(y == m_posy && x >= m_posx + m_sizex - 3 && x < m_posx + m_sizex)
        code = K_DOWN;

    if(m_fListOpen)
      if(y == m_posy)
        code = K_ESC;
  }

  if(m_fListOpen)
    code = m_List.EventProc(code);


  if(!m_fListOpen && code == K_DOWN)
  {
    m_fListOpen = 1;
    m_nSelected = GetSelect();
    m_dcursorx = m_List.m_dcursorx;
    m_dcursory = m_List.m_dcursory;
    m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory + 1);
    m_List.Refresh(CTRL_SELECTED);
    return 0;
  }

  if(m_fListOpen)
  {
    if(code == K_ENTER)
    {
      m_nSelected = GetSelect();
      LostSelect();
      return 0;
    }

    if(code == K_ESC
    || code == K_TAB
    || code == (K_TAB | K_SHIFT)
    || code == K_LEFT
    || code == K_RIGHT
    )
    {
      LostSelect();
      return 0;
    }
  }

  return code;
}


int CtrlSList::Refresh(int type)
{
  //TPRINT(("CtrlSList::Refresh id=%x\n", m_nId));

  int n = GetSelect();
  size_t l;
  char* pStr = GetSelectedStr(n, &l);

  char buff[256];
  if(pStr)
    strcpy_s(buff, sizeof(buff), pStr);
  else
    l = 1;

  size_t d = m_sizex - 3 - --l;//len without last 0
  if(d > 0)
    memset(buff + l, ' ', d);
  buff[m_sizex - 3] = 0;

  color_t color;
  if(type & CTRL_SELECTED)
    color = ColorDialogSelect;
  else if(m_nType & CTRL_DISABLED)
    color = ColorDialogDisabled;
  else
    color = ColorDialogField;

  if(m_nType & CTRL_DISABLED)
    m_dialog.WriteStr(m_posx, m_posy, buff, color);
  else
    m_dialog.WriteStr(m_posx, m_posy, buff, ColorDialogFieldAct);

  m_dialog.WriteStr(m_posx + m_sizex - 3, m_posy, "[v]", color);

  if(m_fListOpen)
    m_List.Refresh(type);

  return 0;
}


int CtrlSList::LostSelect()
{
  //TPRINT(("CtrlSList::LostSelect id=%x\n", m_nId));

  if(m_fListOpen)
  {
    m_fListOpen = 0;
    SetSelect(m_nSelected);
    Select();
    m_dialog.Refresh();
  }

  return 0;
}


int CtrlSList::SetSelect(int n)
{
  m_List.SetSelect(n, 0);
  Refresh();
  return 0;
}


int CtrlSList::CheckMouse(short x, short y)
{
  if(!m_fListOpen)
    return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + 1);
  else
    return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + m_sizey);
}


int CtrlSList::SetPos(short x, short y, short sizex, short sizey)
{
  if(x >= 0)
  {
    m_posx        = x;
    m_List.m_posx = x;
  }

  if(y >= 0)
  {
    m_posy        = y;
    m_List.m_posy = y + 1;
  }

  if(sizex > 0)
  {
    m_sizex           = sizex;
    m_List.m_sizex    = sizex;
  }

  if(sizey > 0)
  {
    m_sizey        = sizey;
    m_List.m_sizey = sizey - 1;
  }

  return 0;
}


size_t CtrlSList::GetName(char*pBuff, size_t len)
{
  int n = m_List.GetSelect();
  size_t l = 0;
  char* p = m_List.GetSelectedStr(n, &l);

  if(pBuff)
  {
    if(p && len > l)
      strncpy_s(pBuff, len, p, len);
    else
      *pBuff = 0;
  }

  return l;
}


/////////////////////////////////////////////////////////////////////////////
CtrlCombo::CtrlCombo(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, pControl->sizex, pControl->sizey, pControl->pHelpLine),
 m_Edit(pDialog, pControl),
 m_List(pDialog, pControl)
{
  m_fListOpen = 0;

  m_Edit.m_sizex    -= 3;
  m_Edit.m_nBuffLen -= 3;
  m_Edit.m_pBuff[m_Edit.m_nBuffLen] = 0;

  m_List.m_sizey -= 1;
  if(m_posy + m_sizey < pDialog->GetWSizeY())
    m_List.m_posy += 1;
  else
    m_List.m_posy -= m_sizey - 1;

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
}


int CtrlCombo::Select()
{
  //TPRINT(("CtrlCombo::Select id=%x\n", m_nId));

  int rc = m_Edit.Select();
  m_dcursorx = m_Edit.m_dcursorx;
  m_dcursory = m_Edit.m_dcursory;

  return rc;
}


int CtrlCombo::UpdateVar()
{
  if(m_pSave)
  {
    char buff[MAX_STRLEN];
    m_Edit.GetName(buff, sizeof(buff));
    m_pSave->AddStr(buff);
  }

  return m_Edit.UpdateVar();
}


int CtrlCombo::EventProc(int code)
{
  //TPRINT(("CtrlCombo::EventProc id=%x code=%x\n", m_nId, code));

  if((code & K_TYPEMASK) == K_MOUSEKUP)
  {
    int x = K_GET_X(code);
    int y = K_GET_Y(code);

    //TPRINT(("Mouse x=%d y=%d\n", x, y));

    if(!m_fListOpen)
      if(y == m_posy && x >= m_posx + m_sizex - 3 && x < m_posx + m_sizex)
        code = K_DOWN;

    if(m_fListOpen)
      if(y == m_posy)
        code = K_ESC;
  }

  if(!m_fListOpen)
    code = m_Edit.EventProc(code);
  else
    code = m_List.EventProc(code);


  if(!m_fListOpen && code == K_DOWN)
  {
    //CaptureInput();
    m_fListOpen = 1;
    m_dcursorx = m_List.m_dcursorx;
    m_dcursory = m_List.m_dcursory + m_List.m_posy - m_posy - 1;
    m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory + 1);
    m_List.Refresh(CTRL_SELECTED);
    return 0;
  }

  if(m_fListOpen)
  {
    if(code == K_ENTER)
    {
      int n = GetSelect();
      size_t l;
      char* pStr = GetSelectedStr(n, &l);
      //TDUMP((pStr, l, "Str=\n"));
      if(pStr)
        m_Edit.SetName(pStr);
    }

    if(code == K_ESC
    || code == K_TAB
    || code == (K_TAB | K_SHIFT)
    || code == K_LEFT
    || code == K_RIGHT
    || code == K_ENTER
    )
    {
      LostSelect();
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


int CtrlCombo::Refresh(int type)
{
  //TPRINT(("CtrlCombo::Refresh id=%x\n", m_nId));

  m_Edit.Refresh(type);

  color_t color;
  if(type & CTRL_SELECTED)
    color = ColorDialogSelect;
  else if(m_nType & CTRL_DISABLED)
    color = ColorDialogDisabled;
  else
    color = ColorDialogField;

  m_dialog.WriteStr(m_posx + m_Edit.m_nBuffLen, m_posy, "[v]", color);

  if(m_fListOpen)
    m_List.Refresh(type);

  return 0;
}


int CtrlCombo::LostSelect()
{
  //TPRINT(("CtrlCombo::LostSelect id=%x\n", m_nId));

  if(m_fListOpen)
  {
    //ReleaseInput();
    m_fListOpen = 0;

    Select();
    m_Edit.m_nEndSel = 0;

    m_dialog.Refresh();
  }

  return 0;
}


int CtrlCombo::CheckMouse(short x, short y)
{
  if(!m_fListOpen)
    return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + 1);
  else
    return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + m_sizey);
}


int CtrlCombo::SetPos(short x, short y, short sizex, short sizey)
{
  if(x >= 0)
  {
    m_posx        = x;
    m_Edit.m_posx = x;
    m_List.m_posx = x;
  }

  if(y >= 0)
  {
    m_posy        = y;
    m_Edit.m_posy = y;
    m_List.m_posy = y + 1;
  }

  if(sizex > 0)
  {
    m_sizex           = sizex;
    m_Edit.m_sizex    = sizex - 3;
    m_Edit.m_nBuffLen = sizex - 3;
    m_Edit.m_pBuff[m_Edit.m_nBuffLen] = 0;
    m_List.m_sizex    = sizex;
  }

  if(sizey > 0)
  {
    m_sizey        = sizey;
    m_List.m_sizey = sizey - 1;
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
CtrlColor::CtrlColor(Dialog* pDialog, control* pControl)
 : Control(pDialog, pControl->type, pControl->pName, pControl->pVar, pControl->id
   , pControl->x, pControl->y, 18, 6, pControl->pHelpLine)
{
  if(m_pVar)
    m_nColor = *((color_t*)m_pVar);
  else
    m_nColor = 0;

  m_nMaxColor = 16;

  SetCursor();
}


color_t CtrlColor::SetVar(color_t c)
{
  if(c != m_nColor)
  {
    PaintSelect(0);
    m_nColor   = c % m_nMaxColor;
    m_dcursorx = 1 + (m_nColor % 4) * 4;
    m_dcursory = 1 + m_nColor / 4;
    PaintSelect(1);
  }
  return m_nColor;
}


int CtrlColor::SetCursor()
{
  m_nColor  %= m_nMaxColor;

  m_dcursorx = 1 + (m_nColor % 4) * 4;
  m_dcursory = 1 + m_nColor / 4;
  m_dialog.GotoXY(m_posx + m_dcursorx, m_posy + m_dcursory);
  return 0;
}


int CtrlColor::PaintSelect(int vis, int sel)
{
  color_t color;
  if(sel)
    color = ColorDialogSelect;
  else if(m_nType & CTRL_DISABLED)
    color = ColorDialogDisabled;
  else
    color = *m_dialog.m_pColorWindow;

  char c;
  if(vis)
    c = '>';
  else
    c = ' ';

  m_dialog.WriteChar(m_posx + m_dcursorx, m_posy + m_dcursory, c, color);

  return 0;
}


int CtrlColor::Refresh(int type)
{
  //TPRINT(("     CtrlColor::Refresh id=%d name=%s\n", m_nId, m_pName));

  color_t color;
  if(m_nType & CTRL_DISABLED)
    color = ColorDialogDisabled;
  else
    color = *m_dialog.m_pColorWindow;

  int size = m_sizex + (strchr(m_pName, '&') ? 1 : 0);
  char* pBuff = new char[size + 1];

  pBuff[0] = ACS_ULCORNER;
  strcpy_s(pBuff + 1, size, m_pName);

  size_t nl = strlen(pBuff);
  memset(pBuff + nl, ACS_HLINE, size - 1 - nl);
  pBuff[size - 1] = ACS_URCORNER;
  pBuff[size]     = 0;
  Paint(pBuff, 0);

  m_dialog.FillRect(m_posx,               m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);
  m_dialog.FillRect(m_posx - 1 + m_sizex, m_posy + 1, 1, m_sizey - 2, ACS_VLINE, color);

  for(short i = 0; i < m_nMaxColor / 4; ++i)
    for(short j = 0; j < 4; ++j)
      m_dialog.FillRect(m_posx + 2 + j * 4, m_posy + 1 + i, 3, 1, ACS_SQUARE, (i * 4 + j) | *m_dialog.m_pColorWindow);

  PaintSelect(1, type);

  pBuff[0] = ACS_LLCORNER;
  memset(pBuff + 1, ACS_HLINE, m_sizex - 2);
  pBuff[m_sizex - 1] = ACS_LRCORNER;
  pBuff[m_sizex]     = 0;
  m_dialog.WriteStr(m_posx, m_posy + m_sizey - 1, pBuff, color);

  delete pBuff;

  return 0;
}


int CtrlColor::EventProc(int code)
{
  color_t color = m_nColor;

  if(code == K_SPACE)
    return 0;
  else if(code == K_LEFT)
  {
    if(--color < 0)
      color = m_nMaxColor - 1;
  }
  else if(code == K_RIGHT)
  {
    if(++color >= m_nMaxColor)
      color = 0;
  }
  else if(code == K_UP)
  {
    color_t c = color / 4;
    if(--c < 0)
      c = m_nMaxColor / 4 - 1;
    color = c * 4 + m_nColor % 4;
  }
  else if(code == K_DOWN)
  {
    color_t c = color / 4;
    if(++c >= m_nMaxColor / 4)
      c = 0;
    color = c * 4 + m_nColor % 4;
  }
  else if(code == K_PAGEUP || code == K_HOME)
    color = 0;
  else if(code == K_PAGEDN || code == K_END)
    color = m_nMaxColor - 1;
  else if(code & K_MOUSE)
  {
    if((code & K_TYPEMASK) == K_MOUSEKUP)
    {
      short x = K_GET_X(code) - m_posx - 1;
      short y = K_GET_Y(code) - m_posy - 1;
      if(x >= 0 && x < m_sizex - 2
      && y >= 0 && y < m_sizey - 2)
        color = x / 4 + y * 4;
    }
  }

  if(color != m_nColor)
  {
    PaintSelect(0);
    m_nColor = color;
    SetCursor();
    PaintSelect(1, 1);
    return K_SELECT + color + 1;
  }

  return code;
}


int CtrlColor::UpdateVar()
{
  if(m_pVar)
    *((int*)m_pVar) = m_nColor;
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
int MsgBox(const char* pTitle, const char* pStr1, const char* pStr2, int type)
{
  short l1 = (short)strlen(GetSStr(pStr1));
  short l2 = (short)strlen(GetSStr(pStr2));
  if(l1 > 70)
    l1 = 70;
  if(l2 > 70)
    l2 = 70;

  short l = l1 > l2 ? l1 : l2;

  if(l < 30)
    l = 30;

  control MBox[] = {
/*0*/ {CTRL_TITLE,                        pTitle,   0,         NULL,  1, 0, l + 4, 8},

/*1*/ {CTRL_STATIC,                       pStr1,    0,         NULL,  1, 1, l1},
/*2*/ {CTRL_STATIC,                       pStr2,    0,         NULL,  1, 2, l2},
/*3*/ {CTRL_LINE,                         "",       0,         NULL,  1, 4, l},

/*4*/ {CTRL_DEFBUTTON | CTRL_ALLIGN_LEFT, "OK",     ID_OK,     NULL,  1, 5},
/*5*/ {CTRL_BUTTON | CTRL_ALLIGN_LEFT,    "Cancel", ID_CANCEL, NULL,  2, 5},
/*6*/ {CTRL_BUTTON | CTRL_ALLIGN_LEFT,    "Ignore", ID_IGNORE, NULL,  3, 5},
      {0}
  };

  switch(type)
  {
  default:
  case MBOX_OK:
    MBox[5].type = 0;
  case MBOX_OK_CANCEL:
    MBox[6].type = 0;
  case MBOX_OK_CANCEL_IGNORE:
    break;
  }

  Dialog Dlg(MBox);
  int rc = Dlg.Activate();

  return rc;
}

#endif