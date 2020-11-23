#pragma once

#include "CaptureInput.h"
#include "KeyCodes.h"
#include "ColorMap.h"

#include <string>

//////////////////////////////////////////////////////////////////////////////
enum class border_t
{
    NO_BORDER     =    0,
    BORDER_TOP    =    1,
    BORDER_LEFT   =    2,
    BORDER_RIGHT  =    4,
    BORDER_BOTTOM =    8,
    BORDER_LINE   = 0x10,
    BORDER_FULL   = 0x1f,
    BORDER_TITLE  = 0x20
};

enum class invrect_t
{
    invFind   = 0,
    invChange = 1,
    invDelete = 2,
    invInsert = 3,
    invFull   = 4
};

class Wnd : public CaptureInput
{
    friend class WndManager;

protected:
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


class FrameWnd : public Wnd
{
    friend class WndManager;

    const color_t* m_pColorWindow       {&ColorWindow};
    const color_t* m_pColorWindowTitle  {&ColorWindowTitle};
    const color_t* m_pColorWindowBorder {&ColorWindowBorder};

protected:
    border_t  m_border  {border_t::NO_BORDER};
    color_t   m_color   {*m_pColorWindow};

public:
    FrameWnd() = default;
    FrameWnd(pos_t left, pos_t top, pos_t sizex, pos_t sizey, border_t border = border_t::NO_BORDER)
    {
        m_left  = left;
        m_top   = top;
        m_sizex = sizex;
        m_sizey = sizey;
        m_border = border;
    }
    virtual ~FrameWnd() = default;

    virtual const std::string   GetWndType() override {return "FWN";}
    virtual bool                Refresh() override;

    virtual void                ClientToScreen(pos_t& x, pos_t& y) override;
    virtual void                ScreenToClient(pos_t& x, pos_t& y) override;
    virtual bool                CheckClientPos(pos_t x, pos_t y) override;
    virtual bool                CheckWndPos(pos_t x, pos_t y) override;

    virtual bool                Repaint() { return true; }
    virtual bool                Invalidate(
        [[maybe_unused]]size_t nline, [[maybe_unused]] invrect_t type, 
        [[maybe_unused]] pos_t pos = 0, [[maybe_unused]] pos_t size = 0)
        {return true;}

    bool  SetBorder(border_t border = border_t::NO_BORDER);
    bool  DrawBorder();

    bool  WriteWnd(pos_t x, pos_t y, const std::string& str, color_t color);
    bool  WriteStr(pos_t x, pos_t y, const std::string& str, color_t color);
    bool  WriteChar(pos_t x, pos_t y, char c, color_t color);

    void  SetTextAttr(color_t color = ColorWindow) { m_color = color; }
    bool  Clr();
    bool  GotoXY(pos_t x, pos_t y);
    bool  WriteStr(pos_t x, pos_t y, const std::u16string& str);
    bool  Scroll(pos_t n, int mode);
    bool  Scroll(pos_t left, pos_t top, pos_t right, pos_t bottom, short n, scroll_t mode);
    bool  FillRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, int c, color_t color);
    bool  ColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, color_t color);
    bool  WriteColor(pos_t x, pos_t y, color_t* pColor);
    bool  WriteColorStr(pos_t x, pos_t y, const std::u16string str, color_t* pColor);
    bool  ShowBuff(pos_t left, pos_t top, pos_t, pos_t);

    void  GetWindowSize(pos_t& sizeX, pos_t& sizeY);

    bool  Beep();
    bool  PutMacro(input_t cmd);
    bool  CheckInput(const std::chrono::milliseconds& waitTime = 100ms);

    pos_t GetWSizeX();
    pos_t GetWSizeY();

protected:
    pos_t GetCSizeX();
    pos_t GetCSizeY();
};
