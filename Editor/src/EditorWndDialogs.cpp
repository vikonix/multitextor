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
#include "WndManager.h"
#include "EditorApp.h"
#include "StdDialogs.h"
#include "Dialogs/EditorDialogs.h"


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
    if (ret == ID_OK)
    {
    }
    return true;
}

bool EditorWnd::DlgReplace([[maybe_unused]] input_t cmd)
{
    FindDialog dlg(true);
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
    }
    return true;
}

bool EditorWnd::SaveAs([[maybe_unused]]input_t cmd)
{
    FileDialog dlg{ FileDlgMode::SaveAs };
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
                    {"Error: File with the same name is opened in editor",
                    "Close it first!"}
                );
                throw std::runtime_error{"File with the same name is opened in editor"};
            }

            if (std::filesystem::exists(path))
            {
                ret = MsgBox(MBoxKey::OK_CANCEL, "Save As",
                    {"File with the same name already exists",
                    "Replace it?"}
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
                LOG(ERROR) << "save as: exception " << ex.what();
                MsgBox(MBoxKey::OK, "Save As",
                    {"File write error",
                    "Check file access and try again"}
                );
                throw std::runtime_error{ "File saveing error" };
            }

            UpdateAccessInfo();
            m_saved = true;
        }
        catch (...)
        {
            LOG(ERROR) << "Error in file Saving As";
            EditorApp::SetErrorLine("Error in file Saving As");
            return false;
        }
    }

    return true;
}

bool EditorWnd::CtrlProperties(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    return true;
}

bool EditorWnd::CtrlChangeCP(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    return true;
}

bool EditorWnd::CtrlFuncList(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    return true;
}

bool EditorWnd::TrackPopupMenu(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    return true;
}
