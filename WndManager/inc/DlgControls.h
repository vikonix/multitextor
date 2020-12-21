/*
FreeBSD License

Copyright (c) 2020 vikonix: valeriy.kovalev.software@gmail.com
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

#include "Dialog.h"


/////////////////////////////////////////////////////////////////////////////
class Control : public CaptureInput
{
    friend class Dialog;

protected:
    Dialog& m_dialog;
    size_t      m_pos;
    std::string m_name;
    std::string m_helpLine;

    std::any    m_var;
    int         m_type;
    int         m_id;
    pos_t       m_posx;
    pos_t       m_posy;
    pos_t       m_sizex;
    pos_t       m_sizey;

    input_t     m_key{};
    pos_t       m_addSize{};
    pos_t       m_dcursorx{};
    pos_t       m_dcursory{};

public:
    explicit Control(Dialog& dialog, size_t pos, int type, const std::string name, std::any var,
        int id, pos_t x, pos_t y, pos_t sizex, pos_t sizey, const std::string helpLine)
        : m_dialog{ dialog }
        , m_pos{ pos }
        , m_name{ name }
        , m_helpLine{ helpLine }
        , m_var{ m_var }
        , m_type{ type }
        , m_id{ id }
        , m_posx{ x }
        , m_posy{ y }
        , m_sizex{ sizex }
        , m_sizey{ sizey }
    {}
    virtual ~Control() = default;

    virtual input_t EventProc(input_t code)
    {
        return code;
    }
    virtual bool Refresh([[maybe_unused]] CtrlState state = CTRL_NORMAL)
    {
        return true;
    }
    virtual bool UpdateVar()
    {
        return true;
    }
    virtual bool CheckMouse(pos_t x, pos_t y)
    {
        return (x >= m_posx && x < m_posx + m_sizex && y >= m_posy && y < m_posy + m_sizey);
    }
    virtual bool Select() { return false; }
    virtual bool LostSelect() { return false; }
    virtual bool SetName(const std::string& name) { m_name = name; return true; }
    virtual const std::string& GetName() { return m_name; }
    virtual bool SetPos(pos_t x = MAX_COORD, pos_t y = MAX_COORD, pos_t sizex = MAX_COORD, pos_t sizey = MAX_COORD)
    {
        if (x != MAX_COORD)     m_posx = x;
        if (y != MAX_COORD)     m_posy = y;
        if (sizex != MAX_COORD) m_sizex = sizex;
        if (sizey != MAX_COORD) m_sizey = sizey;
        return true;
    }

    bool SetHelpLine(const char& help)
    {
        m_helpLine = help; return true;
    }
    int  GetMode()
    {
        return m_type;
    }
    int  SetMode(int mode)
    {
        return m_type |= mode & CTRL_STATE_MASK;
    }
    int  ResetMode(int mode)
    {
        return m_type &= ~(mode & CTRL_STATE_MASK);
    }

    bool Paint(const std::string& str, int type);

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
    CtrlStatic(Dialog& dialog, const control& control);

    virtual input_t EventProc(input_t code) override {return code;}
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool SetName(const std::string& pName) override;
};


/////////////////////////////////////////////////////////////////////////////
class CtrlButton : public Control
{
    friend class Dialog;

public:
    CtrlButton(Dialog& dialog, const control& control);

  virtual input_t EventProc(input_t code) override;
  virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
};


/////////////////////////////////////////////////////////////////////////////
class CtrlCheck : public Control
{
    friend class Dialog;

    bool m_checked;

public:
    CtrlCheck(Dialog& dialog, const control& control);

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

    int  m_index;
    bool m_checked;

public:
    CtrlRadio(Dialog& pDialog, const control& pControl, int index);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;

    bool SetCheck(bool check) { return m_checked = check; }
    bool GetCheck() { return m_checked; }
};

/////////////////////////////////////////////////////////////////////////////
class CtrlGroup : public Control
{
    friend class Dialog;

public:
    CtrlGroup(Dialog& dialog, const control& control);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
};

/////////////////////////////////////////////////////////////////////////////
class CtrlEdit : public Control
{
    friend class Dialog;
    friend class CtrlEditDropList;

    pos_t m_pos;
    pos_t m_beginSel;
    pos_t m_endSel;

    pos_t EndSelect(bool del = false);//false-unselect true-del

public:
    CtrlEdit(Dialog& dialog, const control& control);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;
    virtual bool Select() override;
};

/////////////////////////////////////////////////////////////////////////////
class CtrlList : public Control
{
    friend class Dialog;
    friend class CtrlDropList;
    friend class CtrlEditDropList;

    int     m_selected{};
    int     m_firstLine{};//signed
    input_t m_mouseCmd{};
    bool    m_mouse2{};

public:
    std::vector<std::string> m_list;

    CtrlList(Dialog& dialog, const control& control);
    virtual ~CtrlList();

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;
    virtual bool SetName(const std::string& name) override;

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
    const std::string& GetStr(size_t n)
    {
        if (n >= m_list.size())
            return {};
        return m_list[n];
    }
    int Clear()
    {
        m_selected  = false;
        m_dcursory  = 1;
        m_firstLine = 0;
        m_list.clear();
        return true;
    }

    int GetSelected() {return m_firstLine + m_selected;}
    int SetSelect(int n = -1, bool refresh = true);
};

/////////////////////////////////////////////////////////////////////////////
class CtrlDropList : public Control
{
    CtrlList  m_list;

    bool      m_listOpened{};
    int       m_selected{};

public:
    CtrlDropList(Dialog& dialog, const control& control);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override {return m_list.UpdateVar();}
    virtual bool CheckMouse(pos_t x, pos_t y) override;
    virtual bool Select() override;
    virtual bool LostSelect() override;
    virtual bool SetPos(pos_t x = MAX_COORD, pos_t y = MAX_COORD, pos_t sizex = MAX_COORD, pos_t sizey = MAX_COORD) override;
    virtual bool SetName(const std::string& name) override {return SetSelect(m_list.SetName(name));}
    virtual const std::string& GetName() override;

    //control list
    size_t GetStrCount() {return m_list.GetStrCount();}
    bool AddStr(size_t n, const std::string& str) {return m_list.AddStr(n, str);}
    bool AppendStr(const std::string& str) { return m_list.AppendStr(str); }
    bool ChangeStr(size_t n, std::string& str) { return m_list.ChangeStr(n, str); }
    bool DelStr(size_t n) { return m_list.DelStr(n); }
    const std::string& GetStr(size_t n) { return m_list.GetStr(n); }
    bool Clear() { return m_list.Clear(); }

    int GetSelected() { return m_list.GetSelected(); }
    int SetSelect(int n = -1);
};

/////////////////////////////////////////////////////////////////////////////
class CtrlEditDropList : public Control
{
    CtrlEdit    m_edit;
    CtrlList    m_list;

    bool        m_listOpened{};

public:
    CtrlEditDropList(Dialog& dialog, const control& control);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override { return m_list.UpdateVar(); }
    virtual bool CheckMouse(pos_t x, pos_t y) override;
    virtual bool Select() override;
    virtual bool LostSelect() override;
    virtual bool SetPos(pos_t x = MAX_COORD, pos_t y = MAX_COORD, pos_t sizex = MAX_COORD, pos_t sizey = MAX_COORD) override;
    virtual bool SetName(const std::string& name) override {m_dcursorx = m_edit.m_dcursorx; return m_edit.SetName(name);}

    //control edit
    const std::string& GetName() {return m_edit.GetName();}

    //control list
    size_t GetStrCount() { return m_list.GetStrCount(); }
    bool AddStr(size_t n, const std::string& str) { return m_list.AddStr(n, str); }
    bool AppendStr(const std::string& str) { return m_list.AppendStr(str); }
    bool ChangeStr(size_t n, std::string& str) { return m_list.ChangeStr(n, str); }
    bool DelStr(size_t n) { return m_list.DelStr(n); }
    const std::string& GetStr(size_t n) { return m_list.GetStr(n); }
    bool Clear() { return m_list.Clear(); }

    int GetSelected() { return m_list.GetSelected(); }
    int SetSelect(int n = -1) {return m_list.SetSelect(n);}
};

/////////////////////////////////////////////////////////////////////////////
class CtrlColor : public Control
{
    friend class Dialog;

    color_t m_nColor{};
    size_t  m_nMaxColor{16};

    int SetCursor();
    int PaintSelect(bool vis = 1, bool sel = 0);

public:
    CtrlColor(Dialog& dialog, const control& control);

    virtual input_t EventProc(input_t code) override;
    virtual bool Refresh(CtrlState state = CTRL_NORMAL) override;
    virtual bool UpdateVar() override;

    color_t SetVar(color_t c);
    color_t GetVar() {return m_nColor;}
    size_t SetMaxColor(size_t max) {return m_nMaxColor = max;}
};


/////////////////////////////////////////////////////////////////////////////
int MsgBox(const std::string& title, const std::string& line1, const std::string& line2, int type);
