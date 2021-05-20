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

#include "WndManager/Wnd.h"
#include "Editor.h"
#include "EditorCmd.h"

#include <functional>
#include <unordered_map>

using namespace _WndManager;

namespace _Editor
{

class Diff;

class EditorWnd : public FrameWnd
{
    inline static const size_t FileCheckInterval{ 10 };
    inline static const size_t LogFileCheckInterval{ 1 };

    enum select_state : int
    {
        no          = 0,
        begin       = 1,
        end         = 2,
        visible     = 4,
        begin_vis   = 5,
        complete    = 6
    };
    bool IsSelectStarted()  { return (m_selectState & select_state::begin) != 0; }
    bool IsSelectFinished() { return (m_selectState & select_state::end) != 0; }
    bool IsSelectVisible()  { return (m_selectState & select_state::visible) != 0; }
    bool IsSelectComplete() { return m_selectState == select_state::complete; }

    enum class select_t
    {
        stream,
        line,
        column
    };

    enum class select_change
    {
        clear,
        insert_ch,
        delete_ch,
        insert_str,
        delete_str,
        split_str,
        merge_str
    };

    enum class select_line
    {
        begin,
        full,
        end,
        substr
    };
    
    using EditorFunc = std::function<bool(EditorWnd*, input_t)>;
    static std::unordered_map<EditorCmd, std::pair<EditorFunc, select_state>> s_funcMap;

    EditorPtr       m_editor;
    std::shared_ptr<EditorWnd>  m_clonedWnd;

    bool            m_clone{};
    bool            m_close{};    //close window at EventProc return
    bool            m_deleted{};  //file was deleted by external program
    bool            m_saved{};    //file was saved
    bool            m_untitled{true};
    bool            m_readOnly{};
    bool            m_log{};
    std::chrono::time_point<std::chrono::system_clock> m_checkTime{ 
        std::chrono::system_clock::now() + std::chrono::seconds(1)};

    //file position
    size_t          m_xOffset{};
    size_t          m_firstLine{};
    pos_t           m_clientSizeX{};
    pos_t           m_clientSizeY{};

    //select mode variables
    select_t        m_selectType{select_t::stream};
    select_state    m_selectState{select_state::no};    //current selection state
    bool            m_selectKeyShift{};                 //select with shift key pressed
    bool            m_selectMouse{};                    //select by mouse
    //select coord
    size_t          m_beginX{};
    size_t          m_beginY{};
    size_t          m_endX{};
    size_t          m_endY{};

    //search coord
    size_t          m_foundX{};
    size_t          m_foundY{};
    size_t          m_foundSize{};
    size_t          m_progress{};

    //file position info
    size_t          m_infoStrSize{};

    //invalidate
    bool            m_invalidate{};
    pos_t           m_invBeginX{};
    pos_t           m_invBeginY{};
    pos_t           m_invEndX{};
    pos_t           m_invEndY{};

    bool            m_popupMenu{};

    //lex pair
    int             m_lexX{-1};
    int             m_lexY{-1};

    //diff mode
    std::shared_ptr<Diff>   m_diff;
    int                     m_diffBuff{-1};

    bool    _GotoXY(size_t x, size_t y, bool top = false);
    bool    InvalidateRect(pos_t x = 0, pos_t y = 0, pos_t sizex = 0, pos_t sizey = 0);
    bool    PrintStr(pos_t x, pos_t y, const std::u16string& str, size_t offset, size_t len);

    bool    UpdateAccessInfo();
    bool    UpdateNameInfo();
    bool    UpdatePosInfo();
    bool    UpdateProgress(size_t step);
    bool    UpdateLexPair();
    bool    ChangeSelected(select_change type, size_t line = 0, size_t pos = 0, size_t size = 1);
    bool    CorrectSelection();
    bool    InsertStr(const std::u16string& str, size_t y = STR_NOTDEFINED, bool save = true);
    bool    CopySelected(std::vector<std::u16string>& strArray, select_t& selType);
    bool    PasteSelected(const std::vector<std::u16string>& strArray, select_t selType);
    bool    DelSelected();
    bool    Mark(size_t bx, size_t by, size_t ex, size_t ey, color_t color = 0, select_t selectType = select_t::stream);
    bool    IsNormalSelection(size_t bx, size_t by, size_t ex, size_t ey) const;
    bool    HideFound();
    bool    FindWord(const std::u16string& str, size_t& begin, size_t& end);
    bool    GetSelectedPos(size_t line, size_t& begin, size_t& end, select_line& type) const;
    bool    Find(bool silence = false);
    bool    FindUp(bool silence = false);
    bool    FindDown(bool silence = false);
    bool    IsWord(const std::u16string& str, size_t offset, size_t len);
    bool    CheckFileChanging();
    bool    ReplaceSubstr(size_t line, size_t pos, size_t len, const std::u16string& substr);
    bool    TryDeleteSelectedBlock();

public:
    EditorWnd(pos_t left = 0, pos_t top = 0, pos_t sizex = 0, pos_t sizey = 0, int border = BORDER_TITLE)
        : FrameWnd(left, top, sizex, sizey, border) {m_cmdParser.SetCmdMap(g_defaultEditKeyMap);}
    virtual ~EditorWnd() = default;

    bool        SetFileName(const std::filesystem::path& file, bool untitled = false, 
        const std::string& parseMode = "Text", const std::string& cp = "UTF-8");
    bool        SetEditor(EditorPtr editor);
    EditorPtr   GetEditor() { return m_editor; }
    bool        SelectClear();

    bool        SetDiffMode(std::shared_ptr<Diff> diff = nullptr, int buff = -1)
    {
        m_diff = diff;
        m_diffBuff = buff;
        return true;
    }

    input_t     ParseCommand(input_t cmd);

    virtual bool            Destroy() override;
    virtual input_t         EventProc(input_t code) override;
    virtual bool            Refresh() override;
    virtual bool            Repaint() override;
    virtual bool            Invalidate(size_t line, invalidate_t type, size_t pos = 0, size_t size = 0) override;
    virtual char            GetAccessInfo() const override { return m_editor->GetAccessInfo(); }
    virtual std::string     GetObjectName() const override 
    { 
        return m_editor ? m_editor->GetFilePath().filename().u8string() : ""; 
    }
    virtual bool            IsUsedTimer() const override { return true; }
    virtual bool            IsUsedView() const override { return true; }
    virtual wnd_t           GetWndType() const override { return wnd_t::editor; }
    virtual std::filesystem::path   GetFilePath() const override { return m_editor->GetFilePath(); }
    virtual Wnd*            CloneWnd() override;
    virtual bool            IsClone() const override { return m_clone; }

    bool    IsMarked() { return IsSelectComplete(); }
    bool    IsChanged() { return m_editor->IsChanged(); }
    bool    IsRO() { return m_readOnly; }
    bool    SetRO(bool ro) { return m_readOnly = ro; }
    bool    IsLog() { return m_log; }
    bool    SetLog(bool log) { return m_log = log; }

    bool    EditWndCopy(EditorWnd* from);
    bool    EditWndMove(EditorWnd* from);
    bool    GetWord(std::u16string& buff);
    bool    GetSelectedLines(size_t& begin, size_t& end);

/*
  virtual Wnd*          GetLinkWnd() override   {return m_pTBuff->GetLinkWnd(this);}

  int       IsUntitled()             {return m_fUntitled;}
  long long GetSize()                {return m_pTBuff->GetSize();}

  int       SaveCfg(SSave* pSave);
  int       LoadCfg();

  int       SelectAndGoto(int nline, int size);
  int       ScrollWnd(int iCode, int size = 1);
  size_t    GetCurPos()  {return m_nXOffset + m_cursorx;}
  size_t    GetCurLine() {return m_nFirstLine + m_cursory;}
*/
    ///////////////////////////////////////////////////////////////////////////
    //editor functions
    bool MoveLeft(input_t cmd);
    bool MoveRight(input_t cmd);
    bool MoveScrollLeft(input_t cmd);
    bool MoveScrollRight(input_t cmd);
    bool MoveUp(input_t cmd);
    bool MoveDown(input_t cmd);
    bool MovePageUp(input_t cmd);
    bool MovePageDown(input_t cmd);
    bool MoveFileBegin(input_t cmd);
    bool MoveFileEnd(input_t cmd);
    bool MoveStrBegin(input_t cmd);
    bool MoveStrEnd(input_t cmd);
    bool MoveTabLeft(input_t cmd);
    bool MoveTabRight(input_t cmd);
    bool MoveWordLeft(input_t cmd);
    bool MoveWordRight(input_t cmd);
    bool MovePos(input_t cmd);
    bool MoveCenter(input_t cmd);

    bool SelectWord(input_t cmd);
    bool SelectLine(input_t cmd);
    bool SelectAll(input_t cmd);
    bool SelectBegin(input_t cmd);
    bool SelectEnd(input_t cmd);
    bool SelectUnselect(input_t cmd);
    bool SelectMode(input_t cmd);

    bool EditC(input_t cmd);
    bool EditDelC(input_t cmd);
    bool EditBS(input_t cmd);
    bool EditTab(input_t cmd);
    bool EditEnter(input_t cmd);
    bool EditDelStr(input_t cmd);
    bool EditDelBegin(input_t cmd);
    bool EditDelEnd(input_t cmd);

    bool EditBlockCopy(input_t cmd);
    bool EditBlockMove(input_t cmd);
    bool EditBlockDel(input_t cmd);
    bool EditBlockIndent(input_t cmd);
    bool EditBlockUndent(input_t cmd);
    bool EditCopyToClipboard(input_t cmd);
    bool EditCutToClipboard(input_t cmd);
    bool EditPasteFromClipboard(input_t cmd);
    bool EditUndo(input_t cmd);
    bool EditRedo(input_t cmd);

    bool CtrlFind(input_t cmd);
    bool CtrlFindUp(input_t cmd);
    bool CtrlFindDown(input_t cmd);
    bool FindUpWord(input_t cmd);
    bool FindDownWord(input_t cmd);
    bool Replace(input_t cmd);
    bool Repeat(input_t cmd);

    bool DlgGoto(input_t cmd);
    bool DlgFind(input_t cmd);
    bool DlgReplace(input_t cmd);

    bool CtrlRefresh(input_t cmd);
    bool Reload(input_t cmd);
    bool Save(input_t cmd);
    bool SaveAs(input_t cmd);
    bool Close(input_t cmd);

    bool MoveLexMatch(input_t cmd);
    bool CtrlFuncList(input_t cmd);
    bool CtrlProperties(input_t cmd);
    bool TrackPopupMenu(input_t cmd);
};

} //namespace _Editor
