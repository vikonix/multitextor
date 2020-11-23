#pragma once

#include "CaptureInput.h"
#include "KeyCodes.h"

#include <string>


//////////////////////////////////////////////////////////////////////////////
#define NO_BORDER         0
#define BORDER_TOP        1
#define BORDER_LEFT       2
#define BORDER_RIGHT      4
#define BORDER_BOTTOM     8
#define BORDER_LINE    0x10
#define BORDER_TITLE   0x20
#define BORDER_FULL    0x1f


class Wnd : public CaptureInput
{
    friend class WndManager;

protected:
    std::shared_ptr<WndManager> m_WndManager;

    pos_t m_left {0};
    pos_t m_top {0};
    pos_t m_sizex {0};
    pos_t m_sizey {0};
    pos_t m_cursorx {0};
    pos_t m_cursory {0};
    bool  m_fVisible {false};

public:
    Wnd() = default;
    virtual ~Wnd() {Hide();}

    virtual const std::string       GetWndType()    {return "WND";}
    virtual const std::string       GetObjPath()    {return "...";}
    virtual const std::string       GetObjName()    {return {};}
    virtual char                    GetAccessInfo() {return ' ';}

    virtual bool                    IsClone()       {return false;}
    virtual bool                    IsUsedTimer()   {return false;}
    virtual bool                    IsUsedView()    {return false;}
    virtual std::shared_ptr<Wnd>    CloneWnd()      {return nullptr;}
    virtual std::shared_ptr<Wnd>    GetLinkedWnd()  {return nullptr;}
    virtual input_t                 Close()         {delete this; return K_CLOSE;}
    virtual bool                    Refresh()       {return 0;}
    virtual bool                    CheckWndPos(pos_t /*x*/, pos_t /*y*/) { return false; }

    virtual void                    ClientToScreen(pos_t& x, pos_t& y);
    virtual void                    ScreenToClient(pos_t& x, pos_t& y);
    virtual bool                    CheckClientPos(pos_t x, pos_t y);

    bool    Show(bool refresh = true, int view = 0);
    bool    Hide(bool refresh = true);
    bool    Move(pos_t left, pos_t top, pos_t sizex, pos_t sizey, bool fRefresh = true);

    void    StopPaint();
    void    BeginPaint();
};


/*
class FWnd : public Wnd
{
  friend class WndManager;

public:
  color_t*  m_pColorWindow;
  color_t*  m_pColorWindowTitle;
  color_t*  m_pColorWindowBorder;

protected:
  KeyConv*  m_pKeyConv;

  int       m_border;
  color_t   m_color;

  int       m_Timer;//переменная для отработки таймера

  //указатель на объект связанный с окном

public:
  FWnd();
  FWnd(short left, short top, short sizex, short sizey, int border = NO_BORDER);
  virtual ~FWnd();

  virtual const char*   GetWndType() override {return "FWN";}
  virtual int           Refresh() override;

  virtual void          ClientToScreen(short* pX, short* pY) override;
  virtual void          ScreenToClient(short* pX, short* pY) override;
  virtual int           CheckClientPos(short x, short y) override;
  virtual int           CheckWndPos(short x, short y) override;

  //type 1-change
  //     2-del
  //     3-insert
  virtual int           Invalidate(size_t nline, int type, short pos = 0, short size = -1)
    {(void) nline; (void) type; (void) pos; (void) size; return 0;}
  virtual int           Repaint() {return 0;}

  int   SetKeyConv(int* pConv);

  int   SetBorder(int border = NO_BORDER);
  int   DrawBorder();

  int   WriteWnd(short x, short y, const char* pStr, color_t color);
  int   WriteStr(short x, short y, const char* pStr, color_t color);
  int   WriteChar(short x, short y, char c, color_t color);

  int   Clr();
  int   GotoXY(short x, short y);
  void  SetTextAttr(color_t color = ColorWindow) {m_color = color;}
  int   WriteStr(short x, short y, wchar* pStr);
  int   Scroll(short n, int mode);
  int   Scroll(short left, short top, short right, short bottom, short n, int mode);
  int   FillRect(short left, short top, short sizex, short sizey, int c, color_t color);
  int   ColorRect(short left, short top, short sizex, short sizey, color_t color);
  int   WriteColor(short x, short y, color_t* pColor);
  int   WriteColorStr(short x, short y, wchar* pStr, color_t* pColor);
  int   ShowBuff(short left, short top, short sizex, short sizey);

  void  GetWindowSize(short* pSizeX, short* pSizeY);

  int   Beep() {return g_WndManager->Beep();}
  int   CheckInput(int time = 0) {return g_WndManager->CheckInput(time);}
  int   PutMacro(int cmd) {return g_WndManager->PutMacro(cmd);}
  int   SetTimer(int time = 0);

public:
  short GetWSizeX() {
    if(m_sizex < 0)
      return g_WndManager->GetView(this)->sx + m_sizex + 1 - m_left;
    else
      return m_sizex;
  }

  short GetWSizeY() {
    if(m_sizey < 0)
      return g_WndManager->GetView(this)->sy + m_sizey + 1 - m_top;
    else
      return m_sizey;
  }

protected:
  short GetCSizeX() {
    short size;
    if(m_sizex < 0)
      size = g_WndManager->GetView(this)->sx + m_sizex + 1 - m_left;
    else
      size = m_sizex;
    if(m_border & BORDER_LEFT)  --size;
    if(m_border & BORDER_RIGHT) --size;
    return size;
  }

  short GetCSizeY() {
    short size;
    if(m_sizey < 0)
      size = g_WndManager->GetView(this)->sy + m_sizey + 1 - m_top;
    else
      size = m_sizey;
    if(m_border & (BORDER_TOP | BORDER_TITLE)) --size;
    if(m_border & BORDER_BOTTOM) --size;
    return size;
  }
};

*/
