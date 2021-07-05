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
#include "Console/Types.h"
#include "UndoList.h"
#include "WndManager/Wnd.h"
#include "LexParser.h"

#include <unordered_set>
#include <filesystem>
#include <limits>
#include <algorithm>



using namespace _Utils;
using namespace _WndManager;

namespace _WndManager
{
class FrameWnd;
}
namespace iconvpp
{
class CpConverter;
}

namespace _Editor
{

enum class eol_t
{
    unix_eol,
    win_eol,
    mac_eol
};

enum class file_state
{
    not_changed,
    changed,
    removed
};

#ifdef WIN32
    #define DEF_EOL eol_t::win_eol
#else
    #define DEF_EOL eol_t::unix_eol
#endif

/////////////////////////////////////////////////////////////////////////////
#define STR_NOTDEFINED std::numeric_limits<size_t>::max()

class Editor
{
private:
    std::shared_ptr<iconvpp::CpConverter>       m_converter;

    std::filesystem::path                       m_file;
    std::filesystem::file_time_type             m_fileTime{};
    uintmax_t                                   m_fileSize{};
    MemStrBuff<std::string, std::string_view>   m_buffer;

    std::unordered_set<FrameWnd*>               m_wndList;

    UndoList        m_undoList;
    LexParser       m_lexParser;

    //config variables
    std::string     m_cp{};
    size_t          m_maxStrlen{0x800};
    eol_t           m_eol{DEF_EOL};
    size_t          m_tab{8};   //tab position
    bool            m_saveTab{};//save tab or not
    bool            m_showTab{};
    bool            m_ro{};

    //editor variables
    std::u16string  m_curStrBuff;
    size_t          m_curStr{STR_NOTDEFINED};
    bool            m_curChanged{};

    inline static const size_t c_buffsize{ 0x200000 };//2MB

    bool    ApplyBuffer(const std::shared_ptr<std::array<char, c_buffsize>>& buff, size_t read, size_t& buffOffset,
        std::shared_ptr<StrBuff<std::string, std::string_view>>& strBuff, size_t& strOffset,
        uintmax_t& fileOffset, bool eof);
    bool    FillStrOffset(std::shared_ptr<StrBuff<std::string, std::string_view>> strBuff, size_t size, bool last, size_t& rest);
    bool    ImproveBuff(std::list<std::shared_ptr<StrBuff<std::string, std::string_view>>>::iterator strBuff);

    std::u16string  _GetStr(size_t line, size_t offset, size_t size);
    bool    _AddStr(size_t n, const std::u16string& str);
    bool    AddStr(size_t n, const std::u16string& str);
//    bool    AppendStr(const std::u16string& str);
    bool    ChangeStr(size_t n, const std::u16string& str);
    bool    ConvertStr(const std::u16string& str, std::string& buff) const;

    bool    LoadBuff(uint64_t offset, size_t size, std::shared_ptr<std::string> buff);
    bool    BackupFile();
    bool    Clear();

public:
    Editor(const Editor&) = delete;
    void operator= (const Editor&) = delete;
    
    Editor() = default;
    Editor(const std::filesystem::path& file, const std::string& parseStyle = "", const std::string& cp = "")
        : m_file{file}
    {
        m_lexParser.SetParseStyle(parseStyle);
        m_tab = m_lexParser.GetTabSize();
        m_saveTab = m_lexParser.GetSaveTab();
        m_showTab = m_lexParser.GetShowTab();
        SetCP(cp);
        Clear();
    }

    static size_t UStrLen(const std::u16string& str) 
    {
        auto pos = str.find_last_not_of(' ');
        if (pos == std::string::npos)
        {
            if (!str.empty() && str[0] <= ' ')
                return 0;
            return str.size();
        }
        else
            return pos + 1;
    }

    bool                    SetFilePath(const std::filesystem::path& file);
    std::filesystem::path   GetFilePath() const {return m_file;}

    bool                    LinkWnd(FrameWnd* wnd) { m_wndList.insert(wnd); return true; }
    bool                    UnlinkWnd(FrameWnd* wnd) { m_wndList.erase(wnd); return true; }
    std::list<FrameWnd*>    GetLinkedWnd(FrameWnd* wnd = nullptr) const;
    bool                    InvalidateWnd(size_t line, invalidate_t type, pos_t pos = 0, pos_t size = 0) const;
    bool                    RefreshAllWnd(FrameWnd* wnd) const;

    bool                    Load(bool log = false);
    bool                    LoadTail();
    bool                    Save();
    bool                    SetName(const std::filesystem::path& file, bool copy);
    bool                    ClearModifyFlag();
    char                    GetAccessInfo();
    file_state              CheckFile();
    bool                    IsFileInMemory();

    size_t                  GetMaxStrLen() const    {return m_maxStrlen;}
    void                    SetMaxStrLen(size_t len){m_maxStrlen = std::min(static_cast<size_t>(MAX_STRLEN), len);}
    std::string             GetCP() const           {return m_cp;}
    bool                    SetCP(const std::string& cp);
    eol_t                   GetEol() const          {return m_eol;}
    void                    SetEol(eol_t eol)       {m_eol = eol;}
    size_t                  GetTab() const          {return m_tab;}
    void                    SetTab(size_t tabsize)  {m_tab = tabsize;}
    bool                    GetSaveTab() const      {return m_saveTab;}
    void                    SetSaveTab(bool save)   {m_saveTab = save;}
    bool                    GetShowTab() const      {return m_showTab;}
    void                    SetShowTab(bool show)   {m_lexParser.SetShowTab(m_showTab = show);}

    size_t                  GetStrCount() const {return m_buffer.GetStrCount(); }
    bool                    IsChanged() const {return m_curChanged || m_buffer.IsChanged(); }
    bool                    FlushCurStr();
    uint64_t                GetSize() const {return m_buffer.GetSize(); }
    time_t                  GetModTime() const;// {return m_pDObject->GetTime(); }

    std::u16string          GetStr(size_t line, size_t offset = 0, size_t size = MAX_STRLEN + 1);
    std::u16string          GetStrForFind(size_t line, bool checkCase, bool fast);
    bool                    SetCurStr(size_t line);

    //editor API with undo
    bool                    CorrectTab(bool save, size_t line, std::u16string& str);
    bool                    SaveTab(bool save, size_t line);
    bool                    RestoreTab(bool save, size_t line, const std::u16string& str);

    bool                    AddLine(bool save, size_t line, const std::u16string& str);
    bool                    DelLine(bool save, size_t line, size_t count = 1);
    bool                    MergeLine(bool save, size_t line, size_t pos = MAX_STRLEN + 1, size_t indent = 0);//merge with next
    bool                    SplitLine(bool save, size_t line, size_t pos, size_t indent = 0);

    bool                    AddSubstr(bool save, size_t line, size_t pos, const std::u16string& substr);
    bool                    ChangeSubstr(bool save, size_t line, size_t pos, const std::u16string& substr);
    bool                    ClearSubstr(bool save, size_t line, size_t pos, size_t len);
    bool                    DelSubstr(bool save, size_t line, size_t pos, size_t len);
    bool                    ReplaceSubstr(bool save, size_t line, size_t pos, size_t len, const std::u16string& substr);
    bool                    Indent(bool save, size_t line, size_t pos, size_t len, size_t n);
    bool                    Undent(bool save, size_t line, size_t pos, size_t len, size_t n);

    bool                    AddCh(bool save, size_t line, size_t pos, char16_t ch);
    bool                    ChangeCh(bool save, size_t line, size_t pos, char16_t ch);
    bool                    DelCh(bool save, size_t line, size_t pos)    {return DelSubstr(save, line, pos, 1);}
    bool                    DelBegin(bool save, size_t line, size_t pos) {return DelSubstr(save, line, 0, pos);}
    bool                    DelEnd(bool save, size_t line, size_t pos)   {return ClearSubstr(save, line, pos, m_maxStrlen - pos);}

    //undo control
    void                    SetUndoRemark(const std::string& rem) {m_undoList.SetRemark(rem);}
    bool                    AddUndoCommand(const EditCmd& editCmd, const EditCmd& undoCmd);
    bool                    Command(const EditCmd& cmd);
    std::optional<EditCmd>  GetUndo() { return m_undoList.GetUndoCmd(); }
    std::optional<EditCmd>  GetRedo() { return m_undoList.GetEditCmd(); }
    std::optional<EditCmd>  PeekUndo() { return m_undoList.PeekUndoCmd(); }
    std::optional<EditCmd>  PeekRedo() { return m_undoList.PeekEditCmd(); }

    //lexical API
    bool                    SetParseStyle(const std::string& style);
    std::string             GetParseStyle() const { return m_lexParser.GetParseStyle(); }
    bool                    GetColor(size_t line, const std::u16string& str, std::vector<color_t>& buff, size_t len);
    bool                    CheckLexPair(size_t& line, size_t& pos);
};

using EditorPtr = std::shared_ptr<Editor>;

} //namespace _Editor
