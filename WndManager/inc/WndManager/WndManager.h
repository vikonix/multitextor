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
#pragma once

#include "Console/Console.h"
#include "Wnd.h"

#include <array>
#include <deque>

using namespace _Console;

namespace _WndManager
{

enum class split_t
{
    no_split = 0,
    split_h  = 1,
    split_v  = 2
};

//////////////////////////////////////////////////////////////////////////////
struct Logo
{
    color_t     fillColor{};
    color_t     logoColor{};
    char16_t    fillChar{};
    pos_t       x{};  //left up corner
    pos_t       y{};
    std::list<std::string> logoStr;
};

struct View
{
    pos_t       left  {};
    pos_t       top   {};
    pos_t       sizex {};
    pos_t       sizey {};
    Wnd*        wnd   {};
};

//////////////////////////////////////////////////////////////////////////////
class WndManager final
{
    friend class Application;

protected:
    Console             m_console;
#define CallConsole(p) ((m_disablePaint) ? true : m_console. p)

    //view 0 - dialog (top wnd)
    //view 1/2 - splited view
    std::array<View, 3> m_view {};
    std::deque<Wnd*>    m_wndList;  //windows list sorted in Z order with them activity

    ScreenBuffer        m_screenBuff; //current buffer color/symbol/changing
    Logo                m_logo;

    color_t             m_color         {};
    pos_t               m_cursorx       {};
    pos_t               m_cursory       {};
    cursor_t            m_cursor        {cursor_t::CURSOR_OFF};
    int                 m_disablePaint  {0};
    bool                m_invalidate    {true}; //first paint
    bool                m_invalidTitle  {true};

public:
    //view management
    pos_t               m_splitX{};      //15 minimal
    pos_t               m_splitY{};      //3  minimal
    split_t             m_splitType{};
    int                 m_activeView{};

    pos_t               m_sizex{};       //screen size
    pos_t               m_sizey{};       //
    pos_t               m_topLines{};    //number of line used on top
    pos_t               m_bottomLines{}; //number of line used on bottom

protected:
    WndManager() = default;

public:
    ~WndManager() = default;

    WndManager(const WndManager&) = delete;
    void operator= (const WndManager&) = delete;

    static WndManager& getInstance()
    {
        static WndManager instance;
        return instance;
    }

    bool    Init();
    bool    Deinit();
    bool    SetScreenSize(pos_t sizex = MAX_COORD, pos_t sizey = MAX_COORD) { return m_console.SetScreenSize(sizex, sizey); }
    bool    Resize(pos_t sizex, pos_t sizey);

    input_t CheckInput(const std::chrono::milliseconds& waitTime);
    bool    PutInput(input_t code) { return m_console.PutInput(code); }
    input_t ProcInput(input_t code); //events that not treated will pass to active window
    bool    ShowInputCursor(cursor_t nCursor, pos_t x = -1, pos_t y = -1);
    bool    HideCursor();
    bool    Beep() {return m_console.Beep();}

    bool    Cls();
    bool    Refresh();
    bool    CheckRefresh();
    void    StopPaint()  {++m_disablePaint;}
    void    BeginPaint() { if (m_disablePaint) --m_disablePaint; else { _assert(!"BeginPaint"); } }
    bool    Flush() { return m_console.Flush(); }
    void    SetLogo(const Logo& logo) {m_logo = logo;}
    bool    WriteConsoleTitle(bool set = true);

    bool    IsVisible(const Wnd* wnd);
    bool    AddWnd(Wnd* wnd);
    bool    AddLastWnd(Wnd* wnd);
    bool    DelWnd(Wnd* wnd);
    bool    SetTopWnd(Wnd* pWnd, int view = -1);
    bool    SetTopWnd(int pos, int view = -1);
    size_t  GetWndCount() const;
    Wnd*    GetWnd(size_t pos = 0, int view = -1);

    bool    Show(Wnd* wnd, bool refresh = true, int view = 0);
    bool    Hide(Wnd* wnd, bool refresh = true);

    bool    SetView(pos_t x = 40, pos_t y = 11, split_t type = split_t::no_split);
    bool    ChangeViewMode(int mode = 0);//0-create/del 1-horiz/vert
    bool    CalcView();
    bool    CloneView(Wnd* wnd = nullptr);
    bool    SetActiveView(int n = -1);
    int     GetActiveView() {return m_activeView;}
    bool    TrackView(const std::string& msg);
    const View& GetView(const Wnd* wnd) const;

    void    Invalidate() {m_invalidate = true;}

    ////////////////////////////////////////////////////////////////////////////

    bool    GotoXY(pos_t x, pos_t y);
    bool    SetTextAttr(color_t color);
  
    bool    WriteStr(const std::string& str);
    bool    WriteChar(char c = ' ');
    bool    WriteWChar(char16_t c = ' ');
    bool    WriteWStr(const std::u16string& str);
    bool    WriteColorWStr(const std::u16string& str, const std::vector<color_t>& color);

    bool    Scroll(pos_t left, pos_t top, pos_t right, pos_t bottom, pos_t n, scroll_t mode);
    bool    FillRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, input_t c, color_t color);
    bool    ColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, color_t color);
    bool    InvColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey);
    bool    WriteColor(pos_t x, pos_t y, const std::vector<color_t>& color);

    bool    ShowBuff();
    bool    ShowBuff(pos_t left, pos_t top, pos_t sizex, pos_t sizey);

    bool    GetBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, std::vector<cell_t>& block);
    bool    PutBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, const std::vector<cell_t>& block);

protected:
    bool    WriteBlock(pos_t left, pos_t top, pos_t right, pos_t bottom, const ScreenBuffer& block);
};

} // namespace _WndManager
