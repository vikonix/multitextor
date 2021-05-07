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
#include "utils/Clipboard.h"
#include "EditorWnd.h"
#include "WndManager/WndManager.h"
#include "WndManager/Dialog.h"
#include "EditorApp.h"
#include "Console/KeyCodes.h"
#include "Dialogs/EditorDialogs.h"

namespace _Editor
{

bool EditorWnd::EditC(input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditC " << std::hex << cmd << std::dec;
    TryDeleteSelectedBlock();

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

bool EditorWnd::EditDelC([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditDelC " << std::hex << cmd << std::dec;
    if (TryDeleteSelectedBlock())
    {
        return true;
    }

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

bool EditorWnd::EditBS([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditBS " << std::hex << cmd << std::dec;

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

bool EditorWnd::EditEnter([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditEnter " << std::hex << cmd << std::dec;

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

bool EditorWnd::EditTab([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditTab " << std::hex << cmd << std::dec;
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    size_t t = m_editor->GetTab();
    size_t x1 = (x + t) - (x + t) % t;
    size_t len = x1 - x;
    
    MoveRight(static_cast<input_t>(len));

    bool rc{true};

    if (Application::getInstance().IsInsertMode() && y < m_editor->GetStrCount())
    {
        char16_t ch = m_editor->GetSaveTab() ? S_TAB : ' ';
        std::u16string str;
        str.resize(len, ch);
        
        m_editor->SetUndoRemark("Add tab");
        rc = m_editor->AddSubstr(true, y, x, str);
        ChangeSelected(select_change::insert_ch, y, x, len);
    }

    return rc;
}

bool EditorWnd::EditDelStr([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditDelStr " << std::hex << cmd << std::dec;

    size_t y = m_firstLine + m_cursory;

    m_editor->SetUndoRemark("Del line");
    bool rc = m_editor->DelLine(true, y);
    ChangeSelected(select_change::delete_str, y);
    
    return rc;
}

bool EditorWnd::EditDelBegin([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditDelBegin " << std::hex << cmd << std::dec;
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    if (0 == x)
        return true;

    bool rc = MoveStrBegin(1);

    if (y < m_editor->GetStrCount())
    {
        m_editor->SetUndoRemark("Del begin");
        rc = m_editor->DelBegin(true, y, x);
        ChangeSelected(select_change::delete_ch, y, 0, x);
    }

    return rc;
}

bool EditorWnd::EditDelEnd([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditDelEnd " << std::hex << cmd << std::dec;
    size_t x = m_xOffset + m_cursorx;
    size_t y = m_firstLine + m_cursory;

    bool rc{true};

    if (y < m_editor->GetStrCount())
    {
        m_editor->SetUndoRemark("Del end");
        rc = m_editor->DelEnd(true, y, x);
        ChangeSelected(select_change::delete_ch, y, x, m_editor->GetMaxStrLen() - x);
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

    CorrectSelection();
    _GotoXY(m_beginX, m_beginY);
    bool rc = DelSelected();

    return rc;
}

bool EditorWnd::EditUndo([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditUndo " << std::hex << cmd << std::dec;

    auto editCmd = m_editor->PeekUndo();
    if (!editCmd)
    {
        EditorApp::SetErrorLine("Undo command absents");
        m_editor->SetCurStr(STR_NOTDEFINED);
        if (!m_saved)
            m_editor->ClearModifyFlag();
        return true;
    }

    if (editCmd->line != m_firstLine + m_cursory)
    {
        _GotoXY(editCmd->pos, editCmd->line);
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

        _GotoXY(editCmd->pos, editCmd->line);

        if (editCmd->command == cmd_t::CMD_SET_POS)
            ;//nothing to do
        else if (editCmd->command == cmd_t::CMD_END)
        {
            EditorApp::SetHelpLine("Wait while undo '" + editCmd->remark + "' command");

            int n = 1;
            while ((editCmd = m_editor->GetUndo()) != std::nullopt)
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

    return rc;
}

bool EditorWnd::EditRedo([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    EditRedo " << std::hex << cmd << std::dec;

    auto redoCmd = m_editor->PeekRedo();
    if (!redoCmd)
    {
        EditorApp::SetErrorLine("Redo command absents");
        return true;
    }

    if (redoCmd->line != m_firstLine + m_cursory)
    {
        _GotoXY(redoCmd->pos, redoCmd->line);
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

        _GotoXY(redoCmd->pos, redoCmd->line);

        if (redoCmd->command == cmd_t::CMD_SET_POS)
            ;//nothing to do
        else if (redoCmd->command == cmd_t::CMD_BEGIN)
        {
            EditorApp::SetHelpLine("Wait while redo '" + redoCmd->remark + "' command");

            int n = 1;
            while ((redoCmd = m_editor->GetRedo()) != std::nullopt)
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

    return rc;
}

bool EditorWnd::EditBlockIndent(input_t cmd)
{
    if (m_readOnly)
        return true;
    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditBlockIndent " << std::hex << cmd << std::dec;

    input_t count{ K_GET_CODE(cmd) };
    if (!count)
        count = 1;

    CorrectSelection();

    EditCmd edit { cmd_t::CMD_BEGIN, m_beginY, m_beginX };
    EditCmd undo { cmd_t::CMD_BEGIN, m_beginY, m_beginX };
    m_editor->SetUndoRemark("Indent");
    m_editor->AddUndoCommand(edit, undo);

    size_t n { m_endY - m_beginY };
    bool save {true};
    bool rc {};

    size_t bx, ex;
    size_t i;
    for (i = 0; i <= n; ++i)
    {
        if (m_selectType == select_t::stream)
        {
            if (n == 0)
            {
                bx = m_beginX;
                ex = m_endX;
            }
            else if (i == 0)
            {
                bx = m_beginX;
                ex = m_editor->GetMaxStrLen();
            }
            else if (i == n)
            {
                bx = 0;
                ex = m_endX;
            }
            else
            {
                bx = 0;
                ex = m_editor->GetMaxStrLen();
            }
        }
        else if (m_selectType == select_t::line)
        {
            bx = 0;
            ex = m_editor->GetMaxStrLen();
        }
        else
        {
            bx = m_beginX;
            ex = m_endX;
        }

        size_t y = m_beginY + i;
        size_t size = ex - bx + 1;
        if (size > m_editor->GetMaxStrLen())
            size = m_editor->GetMaxStrLen();

        rc = m_editor->Indent(save, y, bx, size, count);
    }

    edit.command = cmd_t::CMD_END;
    undo.command = cmd_t::CMD_END;
    m_editor->AddUndoCommand(edit, undo);
    
    return rc;
}

bool EditorWnd::EditBlockUndent(input_t cmd)
{
    if (m_readOnly)
        return true;
    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditBlockUndent " << std::hex << cmd << std::dec;

    input_t count{ K_GET_CODE(cmd) };
    if (!count)
        count = 1;

    CorrectSelection();

    EditCmd edit { cmd_t::CMD_BEGIN, m_beginY, m_beginX };
    EditCmd undo { cmd_t::CMD_BEGIN, m_beginY, m_beginX };
    m_editor->SetUndoRemark("Undent");
    m_editor->AddUndoCommand(edit, undo);

    size_t n { m_endY - m_beginY };
    bool save { true };
    bool rc { true };

    size_t bx, ex;
    size_t i;
    for (i = 0; i <= n; ++i)
    {
        if (m_selectType == select_t::stream)
        {
            if (n == 0)
            {
                bx = m_beginX;
                ex = m_endX;
            }
            else if (i == 0)
            {
                bx = m_beginX;
                ex = m_editor->GetMaxStrLen();
            }
            else if (i == n)
            {
                bx = 0;
                ex = m_endX;
            }
            else
            {
                bx = 0;
                ex = m_editor->GetMaxStrLen();
            }
        }
        else if (m_selectType == select_t::line)
        {
            bx = 0;
            ex = m_editor->GetMaxStrLen();
        }
        else
        {
            bx = m_beginX;
            ex = m_endX;
        }

        size_t y = m_beginY + i;
        size_t size = ex - bx + 1;
        if (size > m_editor->GetMaxStrLen())
            size = m_editor->GetMaxStrLen();

        rc = m_editor->Undent(save, y, bx, size, count);
    }

    edit.command = cmd_t::CMD_END;
    undo.command = cmd_t::CMD_END;
    m_editor->AddUndoCommand(edit, undo);
    
    return rc;
}

bool EditorWnd::EditCopyToClipboard(input_t cmd)
{
    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditCopyToClipboard " << std::hex << cmd << std::dec;

    select_t mode;
    std::vector<std::u16string> strArray;
    bool rc = CopySelected(strArray, mode)
    && CopyToClipboard(strArray, mode != select_t::stream ? true : false);

    return rc;
}

bool EditorWnd::EditCutToClipboard(input_t cmd)
{
    if (m_selectState != select_state::complete)
        return true;

    LOG(DEBUG) << "    EditCutToClipboard " << std::hex << cmd << std::dec;

    select_t mode;
    std::vector<std::u16string> strArray;
    bool rc = CopySelected(strArray, mode)
    && DelSelected()
    && CopyToClipboard(strArray, mode != select_t::stream ? true : false);

    return rc;
}

bool EditorWnd::EditPasteFromClipboard(input_t cmd)
{
    if (m_readOnly)
        return true;
    if (!IsClipboardReady())
        return true;

    LOG(DEBUG) << "    EditPasteFromClipboard " << std::hex << cmd << std::dec;
    TryDeleteSelectedBlock();

    std::vector<std::u16string> strArray;
    bool rc = PasteFromClipboard(strArray)
    && PasteSelected(strArray, select_t::stream);
    
    //del mark
    ChangeSelected(select_change::clear);
    return rc;
}

bool EditorWnd::Reload([[maybe_unused]]input_t cmd)
{
    LOG(DEBUG) << "    Reload";

    m_selectState = select_state::no;
    m_selectType = select_t::stream;
    m_beginX = m_endX = 0;
    m_beginY = m_endY = 0;

    bool rc = m_editor->Load();
    rc = Refresh();
    return rc;
}

bool EditorWnd::Close([[maybe_unused]]input_t cmd)
{
    m_close = true;
    return true;
}

bool EditorWnd::CtrlRefresh([[maybe_unused]] input_t cmd)
{
    m_editor->FlushCurStr();
    return Refresh();
}

bool EditorWnd::Replace([[maybe_unused]] input_t cmd)
{
    if (m_readOnly)
        return true;

    //LOG(DEBUG) << "    Replace " << std::hex << cmd << std::dec;

    EditCmd edit{ cmd_t::CMD_BEGIN };
    EditCmd undo{ cmd_t::CMD_BEGIN };

    m_editor->SetUndoRemark("Replace");
    bool undoCmd{};

    auto prevMenu = Application::getInstance().SetAccessMenu(g_replaceMenu);
    WndManager::getInstance().Refresh();

    size_t begin = m_firstLine + m_cursory;
    size_t strCount = m_editor->GetStrCount();
    
    bool userBreak{};
    bool reverce{};
    bool prompt{true};

    auto ProcInput = [this, &userBreak, &prompt, &reverce](bool found) -> bool {
        while(1)
        {
            if (found && !userBreak)
                EditorApp::SetErrorLine("Enter-Replace, Esc-Cancel");
            else
            {
                EditorApp::SetErrorLine("Esc-Cancel");
                Beep();
            }

            input_t code;
            while ((code = CheckInput()) == 0);

            EditorApp::SetHelpLine();

            if (found && code == K_ENTER)
                return true;
            else if (found && (code == 'a' || code == 'A'))
            {
                prompt = false;
                return true;
            }
            else if (code == K_ESC || code == 'q' || code == 'Q')
            {
                userBreak = true;
                break;
            }
            else if (code == 'n' || code == 'N' || code == K_DOWN || code == K_RIGHT)
            {
                reverce = false;
                break;
            }
            else if (code == 'p' || code == 'P' || code == K_UP || code == K_LEFT)
            {
                reverce = true;
                break;
            }
        }

        return false;
    };

    size_t count{};
    size_t progress{};
    size_t fx{}, fy{}, fs{};
    while (1)
    {
        bool found{};
        HideFound();
        if (!reverce)
            found = FindDown(!prompt);
        else
        {
            reverce = false;
            found = FindUp(!prompt);
        }

        if (found)
        {
            fx = m_foundX;
            fy = m_foundY;
            fs = m_foundSize;
        }
        else if (fs != 0)
        {
            //restore last found pos
            m_foundX = fx;
            m_foundY = fy;
            m_foundSize = fs;
            Invalidate(fy, invalidate_t::find, fx, fs);
        }

        if (!found && !prompt)
            break;

        if (prompt)
        {
            Repaint();
            auto replace = ProcInput(found);
            if (!replace)
            {
                if (userBreak)
                    break;
                else
                    continue;
            }
        }

        if (!found)
            break;

        if (!prompt && !undoCmd)
        {
            undoCmd = true;

            edit.line = m_foundY;
            edit.pos  = m_foundX;
            undo.line = m_foundY;
            undo.pos  = m_foundX;
            m_editor->AddUndoCommand(edit, undo);
            EditorApp::SetHelpLine("Replace. Press any key for cancel.");
        }

        if (!prompt)
        {
            if (++progress == 1000)
            {
                progress = 0;
                userBreak = UpdateProgress((m_foundY - begin) * 99 / (strCount - begin));

                if (userBreak)
                    break;
            }
        }

        ++count;
        [[maybe_unused]]bool rc = ReplaceSubstr(m_foundY, m_foundX, FindDialog::s_vars.findStrW.size(), FindDialog::s_vars.replaceStrW);
        
        m_foundSize = FindDialog::s_vars.replaceStrW.size();
        _GotoXY(m_foundX + m_foundSize, m_foundY);
    }

    if (undoCmd)
    {
        edit.command = cmd_t::CMD_END;
        undo.command = cmd_t::CMD_END;
        m_editor->AddUndoCommand(edit, undo);

        if (!count)
            EditorApp::SetErrorLine("String not found");
    }
    
    Application::getInstance().SetAccessMenu(prevMenu);

    if (userBreak)
        EditorApp::SetHelpLine("User Abort", stat_color::grayed);
    else
        EditorApp::SetHelpLine("Ready");

    WndManager::getInstance().Refresh();

    return true;
}

bool EditorWnd::Save(input_t cmd)
{
    input_t force{ K_GET_CODE(cmd) };

    if (force == 0 && !m_editor->IsChanged())
        return true;

    LOG(DEBUG) << "    Save " << std::hex << cmd << std::dec;

    if (m_untitled)
        return SaveAs(0);

    try
    {
        [[maybe_unused]]bool rc = m_editor->Save();
    }
    catch (const std::exception& ex)
    {
        LOG(ERROR) << "save as: exception " << ex.what();
        MsgBox(MBoxKey::OK, "Save",
            { "File write error",
            "Check file access and try again" }
        );
        return false;
    }

    UpdateAccessInfo();
    m_saved = true;

    return true;
}

} //namespace _Editor
