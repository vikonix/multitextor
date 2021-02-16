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

#include "utils/MemBuff.h"
#include "Types.h"
#include "UndoList.h"
#include "Wnd.h"

#include <unordered_set>
#include <filesystem>


enum class eol_t
{
    unix_eol,
    win_eol,
    mac_eol
};

#ifdef WIN32
    #define DEF_EOL eol_t::win_eol
#else
    #define DEF_EOL eol_t::unix_eol
#endif

class FrameWnd;

/////////////////////////////////////////////////////////////////////////////
class Editor
{
private:
    std::filesystem::path                       m_file;
    std::filesystem::file_time_type             m_lastWriteTime;
    MemStrBuff<std::string, std::string_view>   m_buffer;
    std::unordered_set<FrameWnd*>               m_wndList;

    UndoList        m_undoList;
    //LexBuff       m_lexBuff;

    //config variables
    int             m_cp{};
    size_t          m_maxStrlen{MAX_STRLEN};
    eol_t           m_eol{DEF_EOL};
    size_t          m_tab{8};   //tab position
    bool            m_saveTab{};//save tab or not
    bool            m_showTab{};
    bool            m_ro{};

    //editor variables
    size_t          m_curStr{};
    bool            m_curChanged{};
    std::u16string  m_curStrBuff;

    bool    FillStrOffset(std::shared_ptr<StrBuff<std::string, std::string_view>> strBuff, size_t size, bool last, size_t& rest);
    bool    ImproveBuff(std::shared_ptr<StrBuff<std::string, std::string_view>> strBuff);

    std::u16string  _GetStr(size_t line, size_t offset, size_t size);
    bool    _AddStr(size_t n, const std::u16string& str);
    bool    AddStr(size_t n, const std::u16string& str);
    bool    AppendStr(const std::u16string& str);
    bool    ChangeStr(size_t n, const std::u16string& str);
    bool    ConvertStr(const std::u16string& str, std::string& buff) const;

    bool    LoadBuff(uint64_t offset, size_t size, std::shared_ptr<std::string> buff);

public:
    Editor(const Editor&) = delete;
    void operator= (const Editor&) = delete;
    
    Editor() = default;
    Editor(const std::filesystem::path& file, const std::string& parseMode = "", int cp = 0)
        : m_file{file}
        , m_cp{cp}
    {}

    static size_t UStrLen(const std::u16string& str) 
    {
        auto pos = str.find_last_not_of(' ');
        if (pos = std::string::npos)
            return str.size();
        else
            return pos;
    };

    bool                    SetFilePath(const std::filesystem::path& file);
    std::filesystem::path   GetFilePath() const {return m_file;}

    bool                    LinkWnd(FrameWnd* wnd) { m_wndList.insert(wnd); return true; }
    bool                    UnlinkWnd(FrameWnd* wnd) { m_wndList.erase(wnd); return true; }
    std::list<FrameWnd*>    GetLinkedWnd(FrameWnd* wnd = nullptr) const;
    bool                    InvalidateWnd(size_t line, invalidate_t type, pos_t pos = 0, pos_t size = 0) const;
    bool                    RefreshAllWnd(FrameWnd* wnd) const;

    bool                    Clear();
    bool                    Load();
    bool                    Save();
    bool                    SaveAs(const std::string& name);
    bool                    ClearModifyFlag();
    bool                    CheckFileAccess();
    char                    GetAccessInfo();

    size_t                  GetMaxStrLen() const    {return m_maxStrlen;}
    void                    SetMaxStrLen(size_t len){m_maxStrlen = len;}
    int                     GetCP() const           {return m_cp;}
    void                    SetCP(int cp)           {m_cp = cp;}
    eol_t                   GetEol() const          {return m_eol;}
    void                    SetEol(eol_t eol)       {m_eol = eol;}
    size_t                  GetTab() const          {return m_tab;}
    void                    SetTab(size_t tabsize)  {m_tab = tabsize;}
    bool                    GetSaveTab() const      {return m_saveTab;}
    void                    SetSaveTab(bool save)   {m_saveTab = save;}
    bool                    GetShowTab() const      {return m_showTab;}
    void                    SetShowTab(bool show)   {m_showTab = show;}

    size_t                  GetStrCount() const {return m_buffer.GetStrCount(); }
    bool                    IsChanged() const {return m_curChanged || m_buffer.IsChanged(); }
    uint64_t                GetSize() const;// {return m_pDObject->GetSize(); }
    time_t                  GetModTime() const;// {return m_pDObject->GetTime(); }

    //editor API with undo
    std::u16string          GetStr(size_t line, size_t offset = 0, size_t size = MAX_STRLEN);
    bool                    SetCurStr(size_t line);
    size_t                  CalcStrLen(const std::u16string& str);

    bool                    CorrectTab(bool save, size_t line, const std::u16string& str);
    bool                    SaveTab(bool save, size_t line);
    bool                    RestoreTab(bool save, size_t line, const std::u16string& str, size_t len);

    bool                    AddLine(bool save, size_t line, const std::u16string& str);
    bool                    DelLine(bool save, size_t line, size_t count = 1);
    bool                    MergeLine(bool save, size_t line, size_t pos, size_t indent = 0);//merge with next
    bool                    SplitLine(bool save, size_t line, size_t pos, size_t indent = 0);

    bool                    AddSubstr(bool save, size_t line, size_t pos, const std::u16string& substr);
    bool                    ChangeSubstr(bool save, size_t line, size_t pos, const std::u16string& substr);
    bool                    ClearSubstr(bool save, size_t line, size_t pos, size_t len);
    bool                    DelSubstr(bool save, size_t line, size_t pos, size_t len);
    bool                    ReplaceSubstr(bool save, size_t line, size_t pos, size_t len, const std::u16string& substr);
    bool                    Indent(bool save, size_t line, size_t pos, size_t len, size_t n);
    bool                    Undent(bool save, size_t line, size_t pos, size_t len, size_t n);

    bool                    AddCh(bool save, size_t line, int pos, char16_t ch);
    bool                    ChangeCh(bool save, size_t line, int pos, char16_t ch);
    bool                    DelCh(bool save, size_t line, int pos)    {return DelSubstr(save, line, pos, 1);}
    bool                    DelBegin(bool save, size_t line, int pos) {return DelSubstr(save, line, 0, pos);}
    bool                    DelEnd(bool save, size_t line, int pos)   {return ClearSubstr(save, line, pos, MAX_STRLEN);}

    //undo control
    void                    SetUndoRemark(const std::string& rem) {m_undoList.SetRemark(rem);}
    bool                    AddUndoCommand(EditCmd editCmd, EditCmd undoCmd);
    bool                    Command(EditCmd cmd);
    EditCmd                 GetUndo();
    EditCmd                 GetRedo();
    EditCmd                 PeekUndo();
    EditCmd                 PeekRedo();

    //lexical API
    //const char* GetParseMode() { return m_LexBuff.GetParseMode(); }
    bool                    SetParseMode(const std::string& mode);
    bool                    GetColor(size_t line, const std::u16string& str, std::vector<color_t>& buff, size_t len);
    //bool           GetFuncList(List* pList, int* pLine);
    //bool           CheckLexPair(size_t* pLine, int* pX);
};

using EditorPtr = std::shared_ptr<Editor>;

