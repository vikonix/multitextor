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

#include "CaptureInput.h"
#include "KeyCodes.h"
#include "ColorMap.h"
#include "CmdParser.h"
#include "Invalidate.h"

#include <string>

//////////////////////////////////////////////////////////////////////////////
using border_t = uint32_t;
enum enum_border_t
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

enum class wnd_t
{
    wnd     = 1,
    fwnd    = 2,
    menu    = 3,
    dialog  = 4,
    editor  = 5
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
    bool  m_visible {false};

public:
    Wnd() = default;
    virtual ~Wnd() {Hide();}
    virtual input_t                 Destroy()               { return K_CLOSE; };


    virtual wnd_t                   GetWndType() const      {return wnd_t::wnd;}
    virtual const std::wstring      GetObjPath() const      {return L"...";}
    virtual const std::wstring      GetObjName() const      {return {};}
    virtual char                    GetAccessInfo() const   {return ' ';}
    virtual bool                    IsClone() const         {return false;}
    virtual bool                    IsUsedTimer() const     {return false;}
    virtual bool                    IsUsedView() const      {return false;}
    virtual Wnd*                    CloneWnd() const        {return nullptr;}
    virtual Wnd*                    GetLinkedWnd() const    {return nullptr;}

    virtual bool                    Refresh()               {return true;}

    virtual bool                    CheckWndPos(pos_t /*x*/, pos_t /*y*/) const {return false;}
    virtual bool                    CheckClientPos(pos_t x, pos_t y) const;
    virtual void                    ClientToScreen(pos_t& x, pos_t& y) const;
    virtual void                    ScreenToClient(pos_t& x, pos_t& y) const;

    bool    Show(bool refresh = true, int view = 0);
    bool    Hide(bool refresh = true);

    void    StopPaint();
    void    BeginPaint();
};


class FrameWnd : public Wnd
{
friend class Control;
friend class CtrlStatic;

protected:
    CmdParser       m_cmdParser;

    const color_t*  m_pColorWindow      {&ColorWindow};
    const color_t*  m_pColorWindowTitle {&ColorWindowTitle};
    const color_t*  m_pColorWindowBorder{&ColorWindowBorder};

    border_t        m_border            {NO_BORDER};
    color_t         m_color             {*m_pColorWindow};

public:
    FrameWnd() = default;
    FrameWnd(pos_t left, pos_t top, pos_t sizex, pos_t sizey, border_t border = NO_BORDER)
    {
        m_left      = left;
        m_top       = top;
        m_sizex     = sizex;
        m_sizey     = sizey;
        m_border    = border;
    }
    virtual ~FrameWnd() = default;

    virtual wnd_t               GetWndType() const override {return wnd_t::fwnd;}
    virtual bool                Refresh() override;

    virtual bool                CheckWndPos(pos_t x, pos_t y) const override;
    virtual bool                CheckClientPos(pos_t x, pos_t y) const override;
    virtual void                ClientToScreen(pos_t& x, pos_t& y) const override;
    virtual void                ScreenToClient(pos_t& x, pos_t& y) const override;

    virtual bool                Repaint() { return true; }
    virtual bool                Invalidate(
        [[maybe_unused]] size_t line, [[maybe_unused]] invalidate_t type, 
        [[maybe_unused]] size_t pos = 0, [[maybe_unused]] size_t size = 0)
        {return true;}

    bool  SetCmdParser(const CmdMap& cmdMap);
    bool  SetBorder(border_t border = NO_BORDER);
    bool  DrawBorder();

    bool  WriteWnd(pos_t x, pos_t y, const std::string& str, color_t color);
    bool  WriteStr(pos_t x, pos_t y, const std::string& str, color_t color);
    bool  WriteWStr(pos_t x, pos_t y, const std::u16string& str, color_t color = 0);
    bool  WriteChar(pos_t x, pos_t y, char c, color_t color);
    bool  WriteWChar(pos_t x, pos_t y, char16_t c, color_t color);

    void  SetTextAttr(color_t color = ColorWindow) { m_color = color; }
    bool  Clr();
    bool  GotoXY(pos_t x, pos_t y);
    bool  WriteStr(pos_t x, pos_t y, const std::u16string& str);
    bool  Scroll(pos_t n, scroll_t mode);
    bool  Scroll(pos_t left, pos_t top, pos_t right, pos_t bottom, pos_t n, scroll_t mode);
    bool  FillRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, input_t c, color_t color);
    bool  ColorRect(pos_t left, pos_t top, pos_t sizex, pos_t sizey, color_t color);
    bool  WriteColor(pos_t x, pos_t y, const std::vector<color_t>& color);
    bool  WriteColorStr(pos_t x, pos_t y, const std::u16string& str, const std::vector<color_t>& color);
    bool  ShowBuff(pos_t left, pos_t top, pos_t sizex, pos_t sizey);

    void  GetWindowSize(pos_t& sizeX, pos_t& sizeY) const;

    bool  Beep();
    bool  PutMacro(input_t cmd);
    input_t CheckInput(const std::chrono::milliseconds& waitTime = 100ms);

    pos_t GetWSizeX() const;
    pos_t GetWSizeY() const;

protected:
    pos_t GetCSizeX() const;
    pos_t GetCSizeY() const;
};
