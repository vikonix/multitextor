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
#include "Editor.h"
#include "EditorCmd.h"

#include <functional>


class EditorWnd : public FrameWnd
{
    enum class Select
    {
        no,
        begin,
        end
    };

    enum class select_t
    {
        stream,
        line,
        column
    };
    
    using EditorFunc = std::function<bool(EditorWnd*, input_t)>;

    static std::unordered_map<EditorCmd, std::pair<EditorFunc, Select>> s_funcMap;

    EditorPtr   m_editor;

    bool        m_close{};    //close window at EventProc return
    bool        m_deleted{};  //file was deleted by external program
    bool        m_saved{};    //file was saved
    bool        m_clone{};
    bool        m_untitled{true};
    bool        m_readOnly{};
    bool        m_log{};

    //file position
    size_t      m_xOffset{};
    size_t      m_firstLine{};
    pos_t       m_sizeX{};//???
    pos_t       m_sizeY{};//???

    //select mode variables
    //select coord
    select_t    m_selectType{select_t::stream};
    int         m_selectState{};    //current selection state
    bool        m_selectKeyShift{}; //select with shift key pressed
    bool        m_selectMouse{};    //mouse selection
    size_t      m_beginX{};
    size_t      m_beginY{};
    size_t      m_endX{};
    size_t      m_endY{};

    //search coord
    size_t      m_foundX{};
    size_t      m_foundY{};
    size_t      m_foundSize{};

    //file position info
    size_t      m_infoStrSize{};

    //invalidate
    bool        m_invalidate{};
    pos_t       m_invBeginX{};
    pos_t       m_invBeginY{};
    pos_t       m_invEndX{};
    pos_t       m_invEndY{};

    bool        m_popupMenu{};

    //lex pair
    int        m_lexX{-1};
    int        m_lexY{-1};

    //diff mode
    //Diff*      m_pDiff;
    //int        m_nDiffBuff;

    bool    _GotoXY(size_t x, size_t y, bool top = false);
    bool    InvalidateRect(pos_t x = 0, pos_t y = 0, pos_t sizex = 0, pos_t sizey = 0);
    bool    PrintStr(pos_t x, pos_t y, const std::u16string& str, size_t offset, size_t len);

    bool    UpdateAccessInfo();
    bool    UpdateNameInfo();
    bool    UpdatePosInfo();
    bool    UpdateProgress(size_t d);
    bool    UpdateLexPair();

public:
    EditorWnd(pos_t left = 0, pos_t top = 0, pos_t sizex = 0, pos_t sizey = 0, int border = BORDER_TITLE)
        : FrameWnd(left, top, sizex, sizey, border) {m_cmdParser.SetCmdMap(g_defaultEditKeyMap);}
    virtual ~EditorWnd() = default;

    bool        SetFileName(const std::filesystem::path& file, bool untitled = false, const std::string& parseMode = "");
    bool        SetEditor(EditorPtr editor);
    EditorPtr   GetEditor() { return m_editor; }

    bool        Mark(size_t bx, size_t by, size_t ex, size_t ey, color_t color = 0, select_t selectType = select_t::stream);
    bool        IsNormalSelection(size_t bx, size_t by, size_t ex, size_t ey);
    bool        HideFound();

    input_t     ParseCommand(input_t cmd);
    virtual input_t EventProc(input_t code) override;

    virtual bool    Refresh() override;
    virtual bool    Repaint() override;
    virtual bool    Invalidate(size_t line, invalidate_t type, size_t pos = 0, size_t size = 0) override;
    virtual char    GetAccessInfo() const override { return m_editor->GetAccessInfo(); }

    /*

  virtual Wnd*          CloneWnd() override;
  virtual int           IsClone() override      {return m_fClone;}

  virtual const char*   GetWndType() override   {return "EDT";}
  virtual const char*   GetObjPath() override   {return m_pTBuff->GetObjPath();}
  virtual const char*   GetObjName() override   {return m_pTBuff->GetObjName();}
  virtual int           Refresh() override;

  virtual int           IsUsedTimer() override  {return 1;}
  virtual int           IsUsedView() override   {return 1;}
  virtual Wnd*          GetLinkWnd() override   {return m_pTBuff->GetLinkWnd(this);}


  int       IsRO()                   {return m_fReadOnly;}
  int       SetRO(int ro)            {return m_fReadOnly = ro;}
  int       IsLog()                  {return m_fLog;}
  int       SetLog(int log)          {return m_fLog = log;}

  int       IsUntitled()             {return m_fUntitled;}
  int       IsMarked()               {return m_nSelectState;}
  int       IsChanged()              {return m_pTBuff->IsChanged();}
  long long GetSize()                {return m_pTBuff->GetSize();}

  int       SetTextBuff(TextBuff* pTBuff);
  TextBuff* GetTextBuff() {return m_pTBuff;}
  int       SetFileName(char* pName = NULL, int fUntitled = 0, const char* pParse = "");
  int       SaveCfg(SSave* pSave);
  int       LoadCfg();
  int       SetDiffMode(Diff* pDiff = NULL, int diff = -1)
    {m_pDiff = pDiff; m_nDiffBuff = diff; return 0;}

  int       SelectWord(wchar* pStr, size_t* pB, size_t* pE);
  int       GetWord(char* pStr = NULL);
  int       GetSubstr(char* pStr = NULL);

  int       SelectAndGoto(int nline, int size);
  int       Mark(size_t bx, size_t by, size_t ex, size_t ey, color_t color = 0, int SelectType = 0);
  int       IsNormalSelection(size_t bx, size_t by, size_t ex, size_t ey);
  int       ChangeSelected(int cmd, int nline = 0, int pos = 0, int size = 1);
  int       CorrectSelection();
  int       GetSelectedLines(int* pBegin, int* pEnd);

  int       HideFound();
  int       ScrollWnd(int iCode, int size = 1);
  size_t    GetCurPos()  {return m_nXOffset + m_cursorx;}
  size_t    GetCurLine() {return m_nFirstLine + m_cursory;}

  int       ReplaceSubstr(int nline, int pos, int len, wchar* pSubstr, int size);
  int       InsertStr(wchar* pStr, int y = -1, int fSave = 1);
  int       CopySelected(WStrBuff* pBuff, int* pSelType);
  int       PasteSelected(WStrBuff* pBuff, int SelType);
  int       DelSelected();
  int       SelectClear();

  int       EditWndCopy(WndEdit* pWFrom);
  int       EditWndMove(WndEdit* pWFrom);

  int       Find(int fSilence = 0);
  int       FindUp(int fSilence = 0);
  int       FindDown(int fSilence = 0);
  int       IsWord(wchar* pStr, size_t offset, size_t len);
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

    bool EditBlockClear(input_t cmd);
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

    bool Data(input_t cmd);
    bool GotoX(input_t cmd);
    bool GotoY(input_t cmd);

    bool CtrlFind(input_t cmd);
    bool CtrlFindUp(input_t cmd);
    bool CtrlFindDown(input_t cmd);
    bool FindUpWord(input_t cmd);
    bool FindDownWord(input_t cmd);
    bool Replace(input_t cmd);
    bool Again(input_t cmd);

    bool DlgGoto(input_t cmd);
    bool DlgFind(input_t cmd);
    bool DlgReplace(input_t cmd);

    bool CtrlGetSubstr(input_t cmd);
    bool CtrlRefresh(input_t cmd);
    bool Reload(input_t cmd);
    bool Save(input_t cmd);
    bool SaveAs(input_t cmd);
    bool Close(input_t cmd);

    bool MoveLexMatch(input_t cmd);
    bool CtrlFList(input_t cmd);
    bool CtrlProperties(input_t cmd);
    bool CtrlChangeCP(input_t cmd);
    bool TrackPopupMenu(input_t cmd);
};

