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

#include "utfcpp/utf8.h"
#include "Dialog.h"

#include <deque>

/////////////////////////////////////////////////////////////////////////////
namespace _WndManager
{

class Control : public CaptureInput
{
friend class Dialog;

protected:
    Dialog&         m_dialog;

    size_t          m_pos;
    std::u16string  m_name;
    std::string     m_helpLine;

    std::any        m_var;
    int             m_type;
    int             m_id;
    pos_t           m_posx;
    pos_t           m_posy;
    pos_t           m_sizex;
    pos_t           m_sizey;

    input_t         m_key{};
    pos_t           m_addSize{};
    pos_t           m_dcursorx{};
    pos_t           m_dcursory{};

public:
    explicit Control(Dialog& dialog, size_t pos, int type, const std::string name, std::any var,
        int id, pos_t x, pos_t y, pos_t sizex, pos_t sizey, const std::string helpLine)
        : m_dialog{ dialog }
        , m_pos{ pos }
        , m_name{ utf8::utf8to16(name) }
        , m_helpLine{ helpLine }
        , m_var{ var }
        , m_type{ type }
        , m_id{ id }
        , m_posx{ x }
        , m_posy{ y }
        , m_sizex{ sizex }
        , m_sizey{ sizey }
    {}
    virtual ~Control() = default;

    virtual input_t EventProc(input_t code) { return code; }
    virtual bool Refresh([[maybe_unused]] CtrlState state = CTRL_NORMAL) { return true; }
    virtual bool UpdateVar() { return true; }
    virtual bool CheckMouse(pos_t x, pos_t y)
    {
        return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + m_sizey);
    }
    virtual input_t SetFocus() { return K_SELECT; }
    virtual bool LostFocus() { return true; }
    virtual bool SetName(const std::string& name);
    virtual std::string GetName() { return utf8::utf16to8(m_name); }
    virtual std::u16string GetWName() { return m_name; };
    virtual bool SetPos(pos_t x = MAX_COORD, pos_t y = MAX_COORD, pos_t sizex = 0, pos_t sizey = 0)
    {
        if (x != MAX_COORD) m_posx = x;
        if (y != MAX_COORD) m_posy = y;
        if (sizex > 0)      m_sizex = sizex;
        if (sizey > 0)      m_sizey = sizey;
        return true;
    }
    virtual pos_t GetSizeX() { return m_sizex; }

    bool SetHelpLine(const std::string& help) { m_helpLine = help; return true; }
    int  GetMode() { return m_type; }
    int  SetMode(int mode) { return m_type |= mode & CTRL_STATE_MASK; }
    int  ResetMode(int mode) { return m_type &= ~(mode & CTRL_STATE_MASK); }

    bool Paint(const std::u16string& str, int type);

    bool GetPos(pos_t& x, pos_t& y, pos_t& sizex, pos_t& sizey)
    {
        x = m_posx;
        y = m_posy;
        sizex = m_sizex;
        sizey = m_sizey;
        return true;
    }
};

/////////////////////////////////////////////////////////////////////////////
class CtrlStatic : public Control
{
friend class Dialog;

public:
    CtrlStatic(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override {return code;}
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool SetName(const std::string& name) override;
};


/////////////////////////////////////////////////////////////////////////////
class CtrlButton : public Control
{
friend class Dialog;

public:
    CtrlButton(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
};


/////////////////////////////////////////////////////////////////////////////
class CtrlCheck : public Control
{
friend class Dialog;

    bool m_checked{};

public:
    CtrlCheck(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;

    bool SetCheck(bool check) {return m_checked = check;}
    bool GetCheck() {return m_checked;}
};

/////////////////////////////////////////////////////////////////////////////
class CtrlRadio : public Control
{
friend class Dialog;

    size_t m_index{};
    bool m_checked{};

public:
    CtrlRadio(Dialog& dialog, const control& control, size_t pos, size_t index);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;

    bool SetCheck();
    bool GetCheck() { return m_checked; }
};

/////////////////////////////////////////////////////////////////////////////
class CtrlGroup : public Control
{
friend class Dialog;

public:
    CtrlGroup(Dialog& dialog, const control& control, size_t pos);

    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
};

/////////////////////////////////////////////////////////////////////////////
class CtrlEdit : public Control
{
friend class Dialog;
friend class CtrlEditDropList;

    bool            m_selected{};
    size_t          m_offset{0};

    bool Unselect(bool del = false);//false-unselect true-delete

public:
    CtrlEdit(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;
    virtual input_t SetFocus() override;
    virtual bool SetName(const std::string& name) override;
};

/////////////////////////////////////////////////////////////////////////////
class CtrlList : public Control
{
friend class Dialog;
friend class CtrlDropList;
friend class CtrlEditDropList;

    size_t  m_selected{};
    size_t  m_firstLine{};
    input_t m_mouseCmd{};
    bool    m_mouse2{};

public:
    std::deque<std::string> m_list;

    CtrlList(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;
    virtual input_t SetFocus() override { return K_SELECT + (input_t)GetSelected(); }

    size_t GetStrCount() { return m_list.size(); }
    bool AddStr(size_t n, const std::string& str)
    {
        if (n > m_list.size())
            return false;
        m_list.insert(m_list.begin() + n, str); 
        return true;
    }
    bool AppendStr(const std::string& str)
    {
        m_list.push_back(str);
        return true;
    }
    bool ChangeStr(size_t n, const std::string& str)
    {
        if (n >= m_list.size())
            return false;
        m_list[n] = str;
        return true;
    }
    bool DelStr(size_t n)
    {
        if (n >= m_list.size())
            return false;
        m_list.erase(m_list.begin() + n);
        return true;
    }
    const std::string_view GetStr(size_t n)
    {
        if (n >= m_list.size())
            return {};
        return m_list[n];
    }
    int Clear()
    {
        m_firstLine = 0;
        m_selected  = 0;
        m_dcursory  = 1;
        m_list.clear();
        return true;
    }

    size_t GetSelected() {return m_firstLine + m_selected;}
    size_t SetSelect(size_t n, bool refresh = true);
};

/////////////////////////////////////////////////////////////////////////////
class CtrlDropList : public Control
{
    CtrlList  m_list;

    bool      m_listOpened{false};

public:
    CtrlDropList(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override {return m_list.UpdateVar();}
    virtual bool CheckMouse(pos_t x, pos_t y) override;
    virtual input_t SetFocus() override;
    virtual bool LostFocus() override;
    virtual bool SetPos(pos_t x = MAX_COORD, pos_t y = MAX_COORD, pos_t sizex = 0, pos_t sizey = 0) override;
    virtual bool SetName(const std::string& name) override {return SetSelect(m_list.SetName(name));}
    virtual std::string GetName() override;

    //control list
    size_t GetStrCount() {return m_list.GetStrCount();}
    bool AddStr(size_t n, const std::string& str) {return m_list.AddStr(n, str);}
    bool AppendStr(const std::string& str) { return m_list.AppendStr(str); }
    bool ChangeStr(size_t n, std::string& str) { return m_list.ChangeStr(n, str); }
    bool DelStr(size_t n) { return m_list.DelStr(n); }
    const std::string_view GetStr(size_t n) { return m_list.GetStr(n); }
    bool Clear() { return m_list.Clear(); }

    size_t GetSelected() { return m_list.GetSelected(); }
    size_t SetSelect(size_t n);
};

/////////////////////////////////////////////////////////////////////////////
class CtrlEditDropList : public Control
{
    CtrlEdit    m_edit;
    CtrlList    m_list;

    bool        m_listOpened{};

public:
    CtrlEditDropList(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override { return m_edit.UpdateVar(); }
    virtual bool CheckMouse(pos_t x, pos_t y) override;
    virtual input_t SetFocus() override;
    virtual bool LostFocus() override;
    virtual bool SetPos(pos_t x = MAX_COORD, pos_t y = MAX_COORD, pos_t sizex = 0, pos_t sizey = 0) override;
    virtual bool SetName(const std::string& name) override {m_dcursorx = m_edit.m_dcursorx; return m_edit.SetName(name);}

    //control edit
    std::string GetName() override {return m_edit.GetName();}

    //control list
    size_t GetStrCount() { return m_list.GetStrCount(); }
    bool AddStr(size_t n, const std::string& str) { return m_list.AddStr(n, str); }
    bool AppendStr(const std::string& str) { return m_list.AppendStr(str); }
    bool ChangeStr(size_t n, std::string& str) { return m_list.ChangeStr(n, str); }
    bool DelStr(size_t n) { return m_list.DelStr(n); }
    const std::string_view GetStr(size_t n) { return m_list.GetStr(n); }
    bool Clear() { return m_list.Clear(); }

    size_t GetSelected() { return m_list.GetSelected(); }
    size_t SetSelect(size_t n) {return m_list.SetSelect(n);}
};

/////////////////////////////////////////////////////////////////////////////
class CtrlColor : public Control
{
friend class Dialog;

    color_t m_color{};
    color_t m_maxColor{16};

    bool SetCursor();
    bool PaintSelect(bool visible = true, bool selected = false);

public:
    CtrlColor(Dialog& dialog, const control& control, size_t pos);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;

    color_t SetVar(color_t c);
    color_t GetVar() {return m_color;}
    color_t SetMaxColor(color_t max) {return m_maxColor = max;}
};

} // namespace _WndManager
