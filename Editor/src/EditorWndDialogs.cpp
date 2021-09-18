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
#include "utfcpp/utf8.h"
#include "EditorWnd.h"
#include "WndManager/WndManager.h"
#include "EditorApp.h"
#include "Dialogs/EditorDialogs.h"

#include <algorithm>

namespace _Editor
{

bool EditorWnd::DlgGoto([[maybe_unused]]input_t cmd)
{
    GotoDialog dlg(m_editor->GetStrCount());
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        auto line = dlg.GetLine();
        _GotoXY(0, line - 1);
    }
    return true;
}

bool EditorWnd::DlgFind([[maybe_unused]] input_t cmd)
{
    FindDialog dlg(false);
    auto ret = dlg.Activate();
    WndManager::getInstance().CheckRefresh();
    if (ret == ID_OK)
    {
        Find();
    }
    return true;
}

bool EditorWnd::DlgReplace([[maybe_unused]] input_t cmd)
{
    FindDialog dlg(true);
    auto ret = dlg.Activate();
    WndManager::getInstance().CheckRefresh();
    if (ret == ID_OK)
    {
        Replace(0);
    }
    return true;
}

bool EditorWnd::SaveAs([[maybe_unused]]input_t cmd)
{
    FileDialog dlg{ FileDlgMode::SaveAs };
    dlg.s_vars.filepath = m_editor->GetFilePath();
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        try
        {
            std::filesystem::path path{ utf8::utf8to16(dlg.s_vars.path) };
            path /= utf8::utf8to16(dlg.s_vars.file);
            
            auto& app = Application::getInstance();
            auto& editorApp = reinterpret_cast<EditorApp&>(app);

            if (auto wnd = editorApp.GetEditorWnd(path); wnd != nullptr && wnd != this)
            {
                MsgBox(MBoxKey::OK, "Save As",
                    { "Error: File with the same name is opened in editor",
                    "Close it first!" }
                );
                throw std::runtime_error{"File with the same name is opened in editor"};
            }

            if (std::filesystem::exists(path))
            {
                ret = MsgBox(MBoxKey::OK_CANCEL, "Save As",
                    { "File with the same name already exists",
                    "Replace it?" }
                );
                if (ret != ID_OK)
                    return false;
            }
            
            try
            {
                if (!m_untitled)
                {
                    [[maybe_unused]] bool rc = m_editor->Save()
                        && m_editor->SetName(path, true);
                }
                else
                {
                    [[maybe_unused]] bool rc = m_editor->SetName(path, false)
                        && m_editor->Save();
                }
                m_untitled = false;
            }
            catch (const std::exception& ex)
            {
                LOG(ERROR) << __FUNC__ << "save as: exception " << ex.what();
                MsgBox(MBoxKey::OK, "Save As",
                    { "File write error",
                    "Check file access and try again" }
                );
                throw std::runtime_error{ "File saveing error" };
            }

            auto wndList = m_editor->GetLinkedWnd();
            for (auto wnd : wndList)
            {
                auto editorWnd = reinterpret_cast<EditorWnd*>(wnd);
                editorWnd->UpdateAccessInfo();
            }
            m_saved = true;
        }
        catch (...)
        {
            LOG(ERROR) << __FUNC__ << "Error in file Saving As";
            EditorApp::SetErrorLine("Error in file Saving As");
            return false;
        }
    }

    return true;
}

bool EditorWnd::CtrlProperties([[maybe_unused]]input_t cmd)
{
    PropertiesDialog::s_vars.untitled   = m_untitled;
    PropertiesDialog::s_vars.ro         = m_readOnly;
    PropertiesDialog::s_vars.log        = m_log;
    PropertiesDialog::s_vars.cpName     = m_editor->GetCP();
    PropertiesDialog::s_vars.eol        = static_cast<size_t>(m_editor->GetEol());
    PropertiesDialog::s_vars.tabSize    = m_editor->GetTab() - 1;
    PropertiesDialog::s_vars.replaceTab = !m_editor->GetSaveTab();
    PropertiesDialog::s_vars.showTab    = m_editor->GetShowTab();
    PropertiesDialog::s_vars.typeName   = m_editor->GetParseStyle();

    PropertiesDialog dlg;
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        m_editor->FlushCurStr();

        m_readOnly = dlg.s_vars.ro;
        m_log = dlg.s_vars.log;
        m_checkTime = std::chrono::system_clock::now();

        m_editor->SetCP(dlg.s_vars.cpName);
        m_editor->SetEol(static_cast<eol_t>(dlg.s_vars.eol));
        m_editor->SetTab(dlg.s_vars.tabSize + 1);
        m_editor->SetSaveTab(!dlg.s_vars.replaceTab);

        m_editor->SetShowTab(dlg.s_vars.showTab);
        m_editor->SetParseStyle(dlg.s_vars.typeName);//???
    }

    return true;
}

bool EditorWnd::CtrlFuncList(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    Application::getInstance().SetErrorLine("Command not implemented");

    return true;
}

bool EditorWnd::TrackPopupMenu([[maybe_unused]]input_t cmd)
{
    auto maxEntry = std::max_element(g_popupMenu.begin(), g_popupMenu.end(), [](const menu& menu1, const menu& menu2) {
        return menu1.name.size() < menu2.name.size(); });

    pos_t menuX = static_cast<pos_t>(maxEntry->name.size());
    pos_t menuY = static_cast<pos_t>(g_popupMenu.size() + 2);

    pos_t x = m_cursorx ;
    pos_t y = m_cursory + 1;
    if (m_clientSizeX <= menuX)
        x = 0;
    else if (x > m_clientSizeX - menuX)
        x = m_clientSizeX - menuX;
    
    if (m_clientSizeY <= menuY)
        y = 0;
    else if (y > m_clientSizeY - menuY)
        y = m_clientSizeY - menuY;

    ClientToScreen(x, y);
    PopupMenu menu(g_popupMenu, x, y);
    menu.Activate();
    return true;
}

} //namespace _Editor
