#pragma once

#include "Console.h"
#include "Wnd.h"

//////////////////////////////////////////////////////////////////////////////
struct Logo
{
  color_t FillColor;
  color_t LogoColor;
  int     fill;         //fill char
  short   x, y;         //left up corner
  short   nstr;         //number logo str
  char**  ppStr;        //logo str
};


struct View
{
  short left, top;
  short sx,   sy;
  Wnd*  pWnd;
};

//////////////////////////////////////////////////////////////////////////////
class WndManager
{
protected:
  View          m_View[3];
  Wnd*          m_TopWnd;     //список окон отсортированный в Z порядке с учетом активности

  cell_t*         m_TextBuff;   //текущий буффер цвет/символ/изменение
  int           m_BuffSize;
  //сначала все изменения помещаем в новый буффер и потом выводим разницу ???

  const Logo*   m_pLogo;

  int           m_fNotPaint;
  color_t       m_Color;
  short         m_cursorx;
  short         m_cursory;
  int           m_cursor;
  int           m_fInvalidate;
  int           m_fInvTitle;

public:
  Console*     m_Console;

  #define CallConsole(p) ((m_fNotPaint) ? 0 : m_Console-> p)

  //view management
  short         m_nSplitX;      //15 min
  short         m_nSplitY;      //3  min
  int           m_nSplitType;   //0-no_view 1-horiz 2-vert
  int           m_nActiveView;

  short         m_sizex;        //
  short         m_sizey;        //размер экрана
  short         m_TopLine;      //количество линий занятых программой вверху
  short         m_BottomLine;   //количество линий занятых программой внизу

public:
  WndManager();
  ~WndManager();

  int   Init();
  int   Deinit();
  int   Resize(short sizex, short sizey);

  int   CheckInput(int time);
  int   PutMacro(int cmd);
  int   ProcInput(int code);    //обработчик событий что не обработанно передается в активное окно
  int   ShowInputCursor(int nCursor, short x = -1, short y = -1);
  int   HideCursor();
  int   Beep() {return m_Console->Beep();}

  int   Cls();
  int   Refresh();
  int   CheckRefresh();
  void  StopPaint()  {++m_fNotPaint;}
  void  BeginPaint() {--m_fNotPaint;}
  int   Flush();
  int   SetLogo(const Logo * pLogo) {m_pLogo = pLogo; return 0;}

  int   SetConsoleTitle(int fSet = 0);
  int   IsVisible(Wnd* pWnd);
  int   AddWnd(Wnd* wnd);
  int   AddLastWnd(Wnd* wnd);
  int   DelWnd(Wnd* wnd);
  int   SetTopWnd(Wnd* pWnd, int view = -1);
  int   SetTopWnd(int n, int view = -1);
  Wnd*  GetWnd(int n = 0, int view = -1);
  int   GetWndCount();

  int   SetView(short x = 40, short y = 11, int type = 0);
  int   ChangeViewMode(int fType = 0);//0-create/del 1-horiz/vert
  int   CalcView();
  int   CloneView(Wnd* wnd = NULL);
  int   SetActiveView(int n = -1);
  int   GetActiveView() {return m_nActiveView;}
  View* GetView(Wnd* wnd);
  int   TrackView(char* pMsg);

  int   Show(Wnd* wnd, int fRefresh = 1, int view = 0);
  int   Hide(Wnd* wnd, int fRefresh = 1);
  int   Move(Wnd* wnd, int fRefresh = 1);

  int   Invalidate() {return m_fInvalidate = 1;}

  ////////////////////////////////////////////////////////////////////////////

  int   GotoXY(short x, short y);
  int   SetTextAttr(color_t color);
  
  int   WriteStr(const char* pStr);
  int   WriteChar(char c = ' ');
//  int   WriteWStr(wchar* pStr);
//  int   WriteColorWStr(wchar* pStr, color_t* pColor);
//  int   WriteWChar(wchar c = ' ');

  int   Scroll(short left, short top, short right, short bottom, short n, int mode);
  int   FillRect(short left, short top, short sizex, short sizey, int c, color_t color);
  int   ColorRect(short left, short top, short sizex, short sizey, color_t color);
  int   InvColorRect(short left, short top, short sizex, short sizey);
  int   WriteColor(short x, short y, color_t* pColor);

  int   ShowBuff();
  int   ShowBuff(short left, short top, short sizex, short sizey);

  int   GetBlock(cell_t* pBlock, short left, short top, short right, short bottom);
  int   PutBlock(cell_t* pBlock, short left, short top, short right, short bottom);

protected:
  int   WriteBlock(cell_t* pBlock, short left, short top, short right, short bottom);
};

