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

#include "Wnd.h"
#include "WndShade.h"

#include <any>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
namespace _WndManager
{

enum CtrlType
{
    CTRL_END            = 0x0000,
    CTRL_TITLE          = 0x0100,
    CTRL_STATIC         = 0x0200,
    CTRL_LINE           = 0x0300,
    CTRL_GROUP          = 0x0400,
    CTRL_HSCROLL        = 0x1100,
    CTRL_VSCROLL        = 0x1200,
    CTRL_BUTTON         = 0x1300,
    CTRL_DEFBUTTON      = 0x1400,
    CTRL_CHECK          = 0x1500,
    CTRL_RADIO          = 0x1600,
    CTRL_EDIT           = 0x1700,
    CTRL_LIST           = 0x1800,
    CTRL_DROPLIST       = 0x1900,
    CTRL_EDITDROPLIST   = 0x1a00,
    CTRL_COLOR          = 0x1b00,
    CTRL_TYPE_MASK      = 0x1f00
};

enum CtrlState
{
    CTRL_NORMAL         =      0,
    CTRL_DISABLED       =      1,
    CTRL_NOCOLOR        =      2,
    CTRL_HIDE           =      5,
    CTRL_SELECTED       =      8,
    CTRL_SORTED         =   0x10, 
    CTRL_STATE_MASK     = 0x001f
};

enum CtrlAlign
{
    CTRL_ALIGN_RIGHT    = 0x0080,
    CTRL_ALIGN_MASK     = 0x0080
};

enum CtrlDefId
{
    ID_OK               = 0x1000,
    ID_CANCEL           = 0x1001,
    ID_IGNORE           = 0x1002,
    ID_USER             = 0x2000
};

enum class MBoxKey
{
    OK                  = 1,
    OK_CANCEL           = 2,
    OK_CANCEL_IGNORE    = 3
};

/////////////////////////////////////////////////////////////////////////////
struct control
{
    int         type{ CTRL_END };
    std::string name;
    int         id{};
    std::any    var;

    pos_t       x{};
    pos_t       y{};
    pos_t       sizex{};
    pos_t       sizey{};

    std::string helpLine{};
};

/////////////////////////////////////////////////////////////////////////////
class Control;

class Dialog : public FrameWnd
{
friend class Control;
friend class CtrlRadio;
friend class CtrlGroup;
friend class CtrlList;
friend class CtrlColor;

protected:
    //first coltrol in list must be CTRL_TITLE
    std::vector<std::shared_ptr<Control>>   m_controls;
    Shade   m_Shade;

    int     m_activeView{};
    size_t  m_selected{};
    bool    m_saveHelpLine{};
    int     m_mouseKey{};

    void    UpdateVar();

public:
    explicit Dialog(const std::list<control>& controls, pos_t x = MAX_COORD, pos_t y = MAX_COORD);
    virtual ~Dialog() = default;

    virtual wnd_t   GetWndType() const override {return wnd_t::dialog;}
    virtual bool    Refresh() override;

    virtual input_t EventProc(input_t code) override;
    virtual input_t DialogProc(input_t code) {return code;}
    virtual input_t Activate();
    virtual bool    OnActivate() {return true;}
    virtual input_t Close(int id);
    virtual bool    OnClose([[maybe_unused]]int id) {return true;}
    virtual bool    UserPaint() {return true;}

protected:
    bool    AllignButtons();
    bool    _Refresh();

    int     SelectItem(int id);
    input_t  Select(size_t n);

    size_t  GetNextItem();
    size_t  GetPrevItem();
    size_t  GetNextTabItem();
    size_t  GetPrevTabItem();
    int     GetSelectedId();
    std::shared_ptr<Control> GetItem(int id);

    std::pair<bool, size_t> CtrlRadioSelect(size_t pos);
    bool    SaveHelpLine(bool save = true) {return m_saveHelpLine = save;}
};

/////////////////////////////////////////////////////////////////////////////
input_t MsgBox(MBoxKey type, const std::string& title, const std::list<std::string>& message, const std::vector<std::string>& keys = {});

} // namespace _WndManager
