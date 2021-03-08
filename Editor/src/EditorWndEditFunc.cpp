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

#include "utils/SymbolType.h"
#include "EditorWnd.h"
#include "WndManager.h"
#include "EditorApp.h"

bool EditorWnd::EditC(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditC " << std::hex << cmd << std::dec;
    char16_t c = K_GET_CODE(cmd);
    if (c < ' ')
        c = '?';

    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    bool rc = MoveRight(K_ED(E_MOVE_RIGHT));

    if (Application::getInstance().IsInsertMode())
    {
        m_editor->SetUndoRemark("Add char");
        rc = m_editor->AddCh(true, y, x, c);
        ChangeSelected(select_change::insert_ch, y, x, 1);
    }
    else
    {
        m_editor->SetUndoRemark("Change char");
        rc = m_editor->ChangeCh(true, y, x, c);
    }

    return rc;
}

bool EditorWnd::EditDelC(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditDelC " << std::hex << cmd << std::dec;

    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;
    
    bool rc{true};

    if (y < m_editor->GetStrCount())
    {
        auto str = m_editor->GetStr(y);
        size_t len = Editor::UStrLen(str);

        if (x < len)
        {
            //delete current char
            m_editor->SetUndoRemark("Del ch");
            rc = m_editor->DelCh(true, y, x);
            ChangeSelected(select_change::delete_ch, y, x, 1);
        }
        else if (!y && !len)
        {
            m_editor->SetUndoRemark("Del line");
            rc = m_editor->DelLine(true, y);
            ChangeSelected(select_change::delete_str, y);
        }
        else
        {
            m_editor->SetUndoRemark("Merge line");
            rc = m_editor->MergeLine(true, y, x);
            if (!rc)
            {
                Beep();
                EditorApp::SetErrorLine("String too long for merge");
            }
            else
                ChangeSelected(select_change::merge_str, y, x);
        }
    }

    return rc;
}

bool EditorWnd::EditBS(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditBS " << std::hex << cmd << std::dec;

    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    bool rc{true};

    if (y > m_editor->GetStrCount())
    {
        if (x)
            rc = MoveLeft(0);
        else
            rc = MoveUp(0);
    }
    else if (x)
    {
        //delete prev char
        auto str = m_editor->GetStr(y);
        size_t len = Editor::UStrLen(str);
        rc = MoveLeft(0);
        if (x <= len)
        {
            m_editor->SetUndoRemark("Del ch");
            m_editor->DelCh(true, y, x - 1);
            ChangeSelected(select_change::delete_ch, y, x - 1, 1);
        }
    }
    else if (y)
    {
        //merge current line with prev
        rc = MoveUp(0);
        rc = MoveStrEnd(0);
        m_editor->SetUndoRemark("Merge line");
        rc = m_editor->MergeLine(1, y - 1);
        if (!rc)
        {
            Beep();
            EditorApp::SetErrorLine("String too long for merge");
        }
        else
            ChangeSelected(select_change::merge_str, y - 1, m_xOffset + m_cursorx);
    }
    else if (m_editor->GetStrCount() == 1)
    {
        m_editor->SetUndoRemark("Del line");
        rc = m_editor->DelLine(1, y);
        ChangeSelected(select_change::delete_str, y);
    }

    return rc;
}

bool EditorWnd::EditEnter(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditEnter " << std::hex << cmd << std::dec;

    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    auto str = m_editor->GetStr(y);
    auto first = str.find_first_not_of(' ');

    if (first != std::string::npos && x > first)
        //if current string begins from spaces
        _GotoXY(first, y);

    bool rc = MoveDown(0);

    if (Application::getInstance().IsInsertMode() && y < m_editor->GetStrCount())
    {
        m_editor->SetUndoRemark("Split line");
        if (0 == x)
        {
            rc = InsertStr({}, y);
            ChangeSelected(select_change::insert_str, y);
        }
        else
        {
            //cut string
            rc = m_editor->SplitLine(1, y, x, m_xOffset + m_cursorx);
            ChangeSelected(select_change::split_str, y, x, m_xOffset + m_cursorx);
        }
    }

    return rc;
}

bool EditorWnd::EditTab(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditTab " << std::hex << cmd << std::dec;
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    size_t t = m_editor->GetTab();
    size_t x1 = (x + t) - (x + t) % t;
    size_t len = x1 - x;
    
    MoveRight(static_cast<input_t>(len));

    bool rc{true};

    if (Application::getInstance().IsInsertMode() && y < m_editor->GetStrCount())
    {
        char16_t ch = m_editor->GetSaveTab() ? 0x9 : ' ';
        std::u16string str;
        str.resize(len, ch);
        
        m_editor->SetUndoRemark("Add tab");
        rc = m_editor->AddSubstr(true, y, x, str);
        ChangeSelected(select_change::insert_ch, y, x, len);
    }

    return rc;
}

bool EditorWnd::EditDelStr(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditDelStr " << std::hex << cmd << std::dec;

    size_t y = m_firstLine + m_cursory;

    m_editor->SetUndoRemark("Del line");
    bool rc = m_editor->DelLine(true, y);
    ChangeSelected(select_change::delete_str, y);
    
    return rc;
}

bool EditorWnd::EditDelBegin(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditDelBegin " << std::hex << cmd << std::dec;
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    if (0 == x)
        return true;

    bool rc = MoveStrBegin(0);

    if (y < m_editor->GetStrCount())
    {
        m_editor->SetUndoRemark("Del begin");
        rc = m_editor->DelBegin(true, y, x);
        ChangeSelected(select_change::delete_ch, y, 0, x);
    }

    return rc;
}

bool EditorWnd::EditDelEnd(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditDelEnd " << std::hex << cmd << std::dec;
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    bool rc{true};

    if (y < m_editor->GetStrCount())
    {
        m_editor->SetUndoRemark("Del end");
        rc = m_editor->DelEnd(true, y, x);
        ChangeSelected(select_change::delete_ch, y, x, MAX_STRLEN - x);
    }

    return rc;
}

bool EditorWnd::EditBlockCopy(input_t cmd)
{
    if (m_readOnly)
        return true;

    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditBlockCopy " << std::hex << cmd << std::dec << " ss=" << static_cast<int>(m_selectState) << " t=" << static_cast<int>(m_selectType)
        << " bx=" << m_beginX << " by=" << m_beginY << " ex=" << m_endX << " ey=" << m_endY;

    select_t mode;
    std::vector<std::u16string> strArray;
    bool rc = CopySelected(strArray, mode);
    rc = PasteSelected(strArray, mode);

    return rc;
}

bool EditorWnd::EditBlockMove(input_t cmd)
{
    if (m_readOnly)
        return true;

    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditBlockMove " << std::hex << cmd << std::dec << " ss=" << static_cast<int>(m_selectState) << " t=" << static_cast<int>(m_selectType)
        << " bx=" << m_beginX << " by=" << m_beginY << " ex=" << m_endX << " ey=" << m_endY;

    select_t mode;
    std::vector<std::u16string> strArray;

    bool rc = CopySelected(strArray, mode);
    rc = DelSelected();
    rc = PasteSelected(strArray, mode);

    return rc;
}

bool EditorWnd::EditBlockDel(input_t cmd)
{
    if (m_readOnly)
        return true;

    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditBlockDel " << std::hex << cmd << std::dec << " ss=" << static_cast<int>(m_selectState) << " t=" << static_cast<int>(m_selectType)
        << " bx=" << m_beginX << " by=" << m_beginY << " ex=" << m_endX << " ey=" << m_endY;

    bool rc = DelSelected();

    return rc;
}

bool EditorWnd::EditUndo(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditUndo " << std::hex << cmd << std::dec;

    auto editCmd = m_editor->PeekUndo();
    if (!editCmd)
        return true;

    if (editCmd->line != m_firstLine + m_cursory)
    {
        if (editCmd->pos >= 0)//???
            _GotoXY(editCmd->pos, editCmd->line);
        else
            _GotoXY(m_xOffset + m_cursorx, editCmd->line);
        
        return true;
    }

    bool rc{true};
    editCmd = m_editor->GetUndo();
    if (editCmd)
    {
        if (m_selectState == select_state::complete)
        {
            //del mark
            ChangeSelected(select_change::clear);
            InvalidateRect();
        }

        if (editCmd->pos >= 0)//???
            _GotoXY(editCmd->pos, editCmd->line);
        else
            _GotoXY(m_xOffset + m_cursorx, editCmd->line);

        if (editCmd->command == cmd_t::CMD_SET_POS)
            ;//nothing to do
        else if (editCmd->command == cmd_t::CMD_END)
        {
            EditorApp::SetHelpLine("Wait while undo '" + editCmd->remark + "' command");

            int n = 1;
            while (editCmd = m_editor->GetUndo())
            {
                if (editCmd->command == cmd_t::CMD_END)
                    ++n;
                if (editCmd->command == cmd_t::CMD_BEGIN)
                    --n;
                if (!n)
                    break;

                rc = m_editor->Command(*editCmd);
            }

            EditorApp::SetHelpLine("Undo: '" + editCmd->remark + "'", stat_color::grayed);
        }
        else
        {
            EditorApp::SetHelpLine("Undo: '" + editCmd->remark + "'", stat_color::grayed);

            rc = m_editor->Command(*editCmd);
        }
    }
    else
    {
        EditorApp::SetErrorLine("Undo command absents");
        m_editor->SetCurStr(STR_NOTDEFINED);
        if (!m_saved)
            m_editor->ClearModifyFlag();
    }

    return true;
}

bool EditorWnd::EditRedo(input_t cmd)
{
    if (m_readOnly)
        return true;

    LOG(DEBUG) << "    EditRedo " << std::hex << cmd << std::dec;

    auto redoCmd = m_editor->PeekRedo();
    if (!redoCmd)
        return true;

    if (redoCmd->line != m_firstLine + m_cursory)
    {
        if (redoCmd->pos >= 0)//???
            _GotoXY(redoCmd->pos, redoCmd->line);
        else
            _GotoXY(m_xOffset + m_cursorx, redoCmd->line);
        return true;
    }

    bool rc{true};
    redoCmd = m_editor->GetRedo();
    if (redoCmd)
    {
        if (m_selectState == select_state::complete)
        {
            //del mark
            ChangeSelected(select_change::clear);
            InvalidateRect();
        }

        if (redoCmd->pos >= 0)//???
            _GotoXY(redoCmd->pos, redoCmd->line);
        else
            _GotoXY(m_xOffset + m_cursorx, redoCmd->line);

        if (redoCmd->command == cmd_t::CMD_SET_POS)
            ;//nothing to do
        else if (redoCmd->command == cmd_t::CMD_BEGIN)
        {
            EditorApp::SetHelpLine("Wait while redo '" + redoCmd->remark + "' command");

            int n = 1;
            while (redoCmd = m_editor->GetRedo())
            {
                if (redoCmd->command == cmd_t::CMD_BEGIN)
                    ++n;
                if (redoCmd->command == cmd_t::CMD_END)
                    --n;
                if (!n)
                    break;

                rc = m_editor->Command(*redoCmd);
            }

            EditorApp::SetHelpLine("Redo: '" + redoCmd->remark + "'", stat_color::grayed);
        }
        else
        {
            EditorApp::SetHelpLine("Redo: '" + redoCmd->remark + "'", stat_color::grayed);

            rc = m_editor->Command(*redoCmd);
        }
    }
    else
    {
        EditorApp::SetErrorLine("Redo command absents");
    }

    return true;
}

bool EditorWnd::EditBlockIndent(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockUndent(input_t cmd)
{
    return true;
}

bool EditorWnd::EditCopyToClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditCutToClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditPasteFromClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFind(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFindUp(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFindDown(input_t cmd)
{
    return true;
}

bool EditorWnd::FindUpWord(input_t cmd)
{
    return true;
}

bool EditorWnd::FindDownWord(input_t cmd)
{
    return true;
}

bool EditorWnd::Replace(input_t cmd)
{
    return true;
}

bool EditorWnd::Repeat(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgGoto(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgFind(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgReplace(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlGetSubstr(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlRefresh([[maybe_unused]]input_t cmd)
{
    m_editor->FlushCurStr();
    return Refresh();
}

bool EditorWnd::Reload(input_t cmd)
{
    return true;
}

bool EditorWnd::Save(input_t cmd)
{
    return true;
}

bool EditorWnd::SaveAs(input_t cmd)
{
    return true;
}

bool EditorWnd::Close(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlProperties(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlChangeCP(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFuncList(input_t cmd)
{
    return true;
}

bool EditorWnd::TrackPopupMenu(input_t cmd)
{
    return true;
}

